// #include "rehab_robot_ver2_control/robotController.hpp
#include <rclcpp/rclcpp.hpp>
#include <can_odrive_interface/odrive_interface.hpp>
#include "rviz_custom_plugins/msg/kinematics_ik_response.hpp"
#include <vector>
#include <unordered_map>

class IKToMotorBridge : public rclcpp::Node {
public:
    IKToMotorBridge() : Node("ik_to_motor_bridge") {

        odrive_ = std::make_shared<can_odrive_interface::ODriveInterface>();

        this->declare_parameter("motor_node_ids", std::vector<int64_t>{0, 1, 2});
        this->declare_parameter("node1_gear_ratio", 15.0);

        motor_node_ids_ = this->get_parameter("motor_node_ids").as_integer_array();
        node1_gear_ratio_ = this->get_parameter("node1_gear_ratio").as_double();

        for (const auto& node_id : motor_node_ids_) {
            odrive_->InitPositionMode(static_cast<int>(node_id));
            odrive_->SendPositionCommand(static_cast<int>(node_id), 0.0,0,0); // Khởi động về 0 turns
            RCLCPP_INFO(this->get_logger(), "Initialized motor node %ld", node_id);
        }

        // Map khớp theo thứ tự → chỉ số trong joint_positions[]
        joint_index_map_ = {
            {"base_joint",     0},
            {"shoulder_joint", 1},
            {"elbow_joint",    2}
        };

        // SUBSCRIBE topic mới
        ik_sub_ = this->create_subscription<rviz_custom_plugins::msg::KinematicsIKResponse>(
            "/InverseKinematics/response", 10,
            std::bind(&IKToMotorBridge::ik_callback, this, std::placeholders::_1));

        RCLCPP_INFO(this->get_logger(), "IK to Motor Bridge ready. Listening on /InverseKinematics/response");
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

            // Lấy góc đúng theo tên khớp
            std::string joint_name = msg->joint_names[i];
            double joint_angle_rad = msg->joint_positions[joint_index_map_[joint_name]];

            // Convert rad → motor turns
            float turns = joint_angle_rad / REV_TO_RAD;

            // Áp dụng gear ratio cho motor node 1 (shoulder)
            if (node_id == 1)
                turns *= node1_gear_ratio_;

            odrive_->SendPositionCommand(node_id, turns,0,0);

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
    rclcpp::spin(std::make_shared<IKToMotorBridge>());
    rclcpp::shutdown();
    return 0;
}
