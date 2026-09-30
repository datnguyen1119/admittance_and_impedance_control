#include <rclcpp/rclcpp.hpp>
#include <rclcpp_lifecycle/lifecycle_node.hpp>
#include <can_odrive_interface/odrive_interface.hpp>
#include "rviz_custom_plugins/msg/kinematics_ik_response.hpp"
#include <vector>
#include <unordered_map>
#include <memory>

using CallbackReturn = rclcpp_lifecycle::node_interfaces::LifecycleNodeInterface::CallbackReturn;

class IKToMotorBridge : public rclcpp_lifecycle::LifecycleNode {
public:
    IKToMotorBridge() : LifecycleNode("ik_to_motor_bridge") {}

    CallbackReturn on_configure(const rclcpp_lifecycle::State &state) override {
        RCLCPP_INFO(this->get_logger(), "Configuring IKToMotorBridge");

        // Declare parameters
        this->declare_parameter("motor_node_ids", std::vector<int64_t>{0, 1, 2});
        this->declare_parameter("node1_gear_ratio", 15.0);

        motor_node_ids_ = this->get_parameter("motor_node_ids").as_integer_array();
        node1_gear_ratio_ = this->get_parameter("node1_gear_ratio").as_double();

        // Joint index map
        joint_index_map_ = {
            {"base_joint",     0},
            {"shoulder_joint", 1},
            {"elbow_joint",    2}
        };

        // Initialize ODrive
        odrive_ = std::make_shared<can_odrive_interface::ODriveInterface>();

        RCLCPP_INFO(this->get_logger(), "IKToMotorBridge configured");
        return CallbackReturn::SUCCESS;
    }

    CallbackReturn on_activate(const rclcpp_lifecycle::State &state) override {
        RCLCPP_INFO(this->get_logger(), "Activating IKToMotorBridge");

        // Initialize motors
        for (const auto& node_id : motor_node_ids_) {
            odrive_->InitPositionMode(static_cast<int>(node_id));
            odrive_->SendPositionCommand(static_cast<int>(node_id), 0.0, 0, 0); // Initialize to 0 turns
            RCLCPP_INFO(this->get_logger(), "Initialized motor node %ld", node_id);
        }

        // Create subscription
        ik_sub_ = this->create_subscription<rviz_custom_plugins::msg::KinematicsIKResponse>(
            "/InverseKinematics/response", 10,
            std::bind(&IKToMotorBridge::ik_callback, this, std::placeholders::_1));

        RCLCPP_INFO(this->get_logger(), "IK to Motor Bridge ready. Listening on /InverseKinematics/response");
        return CallbackReturn::SUCCESS;
    }

    CallbackReturn on_deactivate(const rclcpp_lifecycle::State &state) override {
        RCLCPP_INFO(this->get_logger(), "Deactivating IKToMotorBridge");

        // Reset subscription
        ik_sub_.reset();

        return CallbackReturn::SUCCESS;
    }

    CallbackReturn on_cleanup(const rclcpp_lifecycle::State &state) override {
        RCLCPP_INFO(this->get_logger(), "Cleaning up IKToMotorBridge");

        odrive_.reset();
        joint_index_map_.clear();

        return CallbackReturn::SUCCESS;
    }

    CallbackReturn on_shutdown(const rclcpp_lifecycle::State &state) override {
        RCLCPP_INFO(this->get_logger(), "Shutting down IKToMotorBridge");
        return CallbackReturn::SUCCESS;
    }

private:
    void ik_callback(const rviz_custom_plugins::msg::KinematicsIKResponse::SharedPtr msg) {
        if (msg->status != "ok") {
            RCLCPP_WARN(this->get_logger(), "IK failed: %s", msg->status.c_str());
            return;
        }

        if (msg->joint_positions.size() < 3) {
            RCLCPP_ERROR(this->get_logger(), "Invalid joint position array!");
            return;
        }

        const double REV_TO_RAD = M_PI / 4.0;

        for (size_t i = 0; i < motor_node_ids_.size(); i++) {
            int node_id = static_cast<int>(motor_node_ids_[i]);

            // Get joint angle by name
            std::string joint_name = msg->joint_names[i];
            double joint_angle_rad = msg->joint_positions[joint_index_map_[joint_name]];

            // Convert rad → motor turns
            float turns = joint_angle_rad / REV_TO_RAD;

            // Apply gear ratio for motor node 1 (shoulder)
            if (node_id == 1)
                turns *= node1_gear_ratio_;

            odrive_->SendPositionCommand(node_id, turns, 0, 0);

            RCLCPP_INFO(this->get_logger(),
                "[%s → Motor %d] %.4f rad → %.4f turns",
                joint_name.c_str(), node_id, joint_angle_rad, turns);
        }
    }

    std::shared_ptr<can_odrive_interface::ODriveInterface> odrive_;
    rclcpp::Subscription<rviz_custom_plugins::msg::KinematicsIKResponse>::SharedPtr ik_sub_;
    std::vector<int64_t> motor_node_ids_;
    double node1_gear_ratio_;
    std::unordered_map<std::string, int> joint_index_map_;
};

int main(int argc, char *argv[]) {
    rclcpp::init(argc, argv);
    auto node = std::make_shared<IKToMotorBridge>();
    rclcpp::spin(node->get_node_base_interface());
    rclcpp::shutdown();
    return 0;
}