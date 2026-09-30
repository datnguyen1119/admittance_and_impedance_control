#include <rclcpp/rclcpp.hpp>
#include <rclcpp_lifecycle/lifecycle_node.hpp>
#include <std_msgs/msg/float64.hpp>
#include <can_odrive_interface/odrive_interface.hpp>
#include <vector>
#include <memory>

using CallbackReturn = rclcpp_lifecycle::node_interfaces::LifecycleNodeInterface::CallbackReturn;

class MotorStatePublisher : public rclcpp_lifecycle::LifecycleNode {
public:
    MotorStatePublisher() : LifecycleNode("motor_state_publisher") {}

    CallbackReturn on_configure(const rclcpp_lifecycle::State &state) override {
        RCLCPP_INFO(this->get_logger(), "Configuring MotorStatePublisher");

        // Declare parameters
        this->declare_parameter("motor_node_ids", std::vector<int64_t>{0, 1, 2});
        this->declare_parameter("node1_gear_ratio", 15.0);
        this->declare_parameter("publish_rate", 10.0);

        motor_node_ids_ = this->get_parameter("motor_node_ids").as_integer_array();
        node1_gear_ratio_ = this->get_parameter("node1_gear_ratio").as_double();
        double publish_rate = this->get_parameter("publish_rate").as_double();

        // Joint names
        joint_names_ = {"base_joint", "shoulder_joint", "elbow_joint"};

        // Create publishers
        for (const auto &name : joint_names_) {
            pos_pubs_[name] = this->create_publisher<std_msgs::msg::Float64>("/" + name + "/m_pos", 10);
            vel_pubs_[name] = this->create_publisher<std_msgs::msg::Float64>("/" + name + "/m_vel", 10);
        }

        // Initialize ODrive
        odrive_ = std::make_shared<can_odrive_interface::ODriveInterface>();

        RCLCPP_INFO(this->get_logger(), "MotorStatePublisher configured");
        return CallbackReturn::SUCCESS;
    }

    CallbackReturn on_activate(const rclcpp_lifecycle::State &state) override {
        RCLCPP_INFO(this->get_logger(), "Activating MotorStatePublisher");

        // Activate publishers
        for (auto &pub : pos_pubs_) {
            pub.second->on_activate();
        }
        for (auto &pub : vel_pubs_) {
            pub.second->on_activate();
        }

        double publish_rate = this->get_parameter("publish_rate").as_double();

        // Create timers
        request_timer_ = this->create_wall_timer(
            std::chrono::milliseconds(static_cast<int>(1000.0 / publish_rate)),
            std::bind(&MotorStatePublisher::request_encoder_data, this));

        publish_timer_ = this->create_wall_timer(
            std::chrono::milliseconds(static_cast<int>(1000.0 / publish_rate) + 10),
            std::bind(&MotorStatePublisher::publish_joint_state_topics, this));

        RCLCPP_INFO(this->get_logger(), "Publishing separate joint state topics at %.1f Hz", publish_rate);
        return CallbackReturn::SUCCESS;
    }

    CallbackReturn on_deactivate(const rclcpp_lifecycle::State &state) override {
        RCLCPP_INFO(this->get_logger(), "Deactivating MotorStatePublisher");

        // Deactivate publishers
        for (auto &pub : pos_pubs_) {
            pub.second->on_deactivate();
        }
        for (auto &pub : vel_pubs_) {
            pub.second->on_deactivate();
        }

        // Reset timers
        request_timer_.reset();
        publish_timer_.reset();

        return CallbackReturn::SUCCESS;
    }

    CallbackReturn on_cleanup(const rclcpp_lifecycle::State &state) override {
        RCLCPP_INFO(this->get_logger(), "Cleaning up MotorStatePublisher");

        // Clear publishers
        pos_pubs_.clear();
        vel_pubs_.clear();
        odrive_.reset();

        return CallbackReturn::SUCCESS;
    }

    CallbackReturn on_shutdown(const rclcpp_lifecycle::State &state) override {
        RCLCPP_INFO(this->get_logger(), "Shutting down MotorStatePublisher");
        return CallbackReturn::SUCCESS;
    }

private:
    void request_encoder_data() {
        if (odrive_) {
            odrive_->ReadEncoder();
        }
    }

    void publish_joint_state_topics() {
        if (!odrive_) return;

        const double REV_TO_RAD = M_PI / 4.0;

        for (size_t i = 0; i < motor_node_ids_.size(); i++) {
            int node_id = static_cast<int>(motor_node_ids_[i]);
            std::string joint = joint_names_[i];

            float turns = 0.0;
            float vel_turns = 0.0;
            odrive_->get_encoder_data(node_id, turns, vel_turns);

            // Convert turns → rad
            float angle_rad, vel_rad;

            if (node_id == 1) {
                angle_rad = (turns / node1_gear_ratio_) * REV_TO_RAD;
                vel_rad = (vel_turns / node1_gear_ratio_) * REV_TO_RAD;
            } else {
                angle_rad = turns * REV_TO_RAD;
                vel_rad = vel_turns * REV_TO_RAD;
            }

            // Publish
            std_msgs::msg::Float64 pos_msg;
            pos_msg.data = angle_rad;
            pos_pubs_[joint]->publish(pos_msg);

            std_msgs::msg::Float64 vel_msg;
            vel_msg.data = vel_rad;
            vel_pubs_[joint]->publish(vel_msg);
        }
    }

    std::shared_ptr<can_odrive_interface::ODriveInterface> odrive_;
    rclcpp::TimerBase::SharedPtr request_timer_;
    rclcpp::TimerBase::SharedPtr publish_timer_;

    std::vector<int64_t> motor_node_ids_;
    double node1_gear_ratio_;

    std::vector<std::string> joint_names_;
    std::unordered_map<std::string, rclcpp::Publisher<std_msgs::msg::Float64>::SharedPtr> pos_pubs_;
    std::unordered_map<std::string, rclcpp::Publisher<std_msgs::msg::Float64>::SharedPtr> vel_pubs_;
};

int main(int argc, char *argv[]) {
    rclcpp::init(argc, argv);
    auto node = std::make_shared<MotorStatePublisher>();
    rclcpp::spin(node->get_node_base_interface());
    rclcpp::shutdown();
    return 0;
}