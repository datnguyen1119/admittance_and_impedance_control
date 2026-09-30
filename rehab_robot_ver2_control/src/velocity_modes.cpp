#include <rclcpp/rclcpp.hpp>
#include <can_odrive_interface/odrive_interface.hpp>
#include <vector>
#include <string>
#include <std_msgs/msg/string.hpp>


class VelocityModesNode : public rclcpp::Node {
public:
    VelocityModesNode() : Node("velocity_config_node") {
        // Declare parameter
        this->declare_parameter<std::vector<std::string>>("control_mode", {"slow","normal","fast"});
        this->declare_parameter<std::vector<double>>("slow_vel", {0.0,0.0,0.0});
        this->declare_parameter<std::vector<double>>("slow_accel", {0.0,0.0,0.0});
        this->declare_parameter<std::vector<double>>("slow_decel", {0.0,0.0,0.0});
        this->declare_parameter<std::vector<double>>("normal_vel", {0.0,0.0,0.0});
        this->declare_parameter<std::vector<double>>("normal_accel", {0.0,0.0,0.0});
        this->declare_parameter<std::vector<double>>("normal_decel", {0.0,0.0,0.0});
        this->declare_parameter<std::vector<double>>("fast_vel", {0.0,0.0,0.0});
        this->declare_parameter<std::vector<double>>("fast_accel", {0.0,0.0,0.0});
        this->declare_parameter<std::vector<double>>("fast_decel", {0.0,0.0,0.0});

        // Lấy các tham số
        // control_mode_ = this->get_parameter("control_mode").as_string_array();
        slow_vel_    = this->get_parameter("slow_vel").as_double_array();
        slow_accel_  = this->get_parameter("slow_accel").as_double_array();
        slow_decel_  = this->get_parameter("slow_decel").as_double_array();

        normal_vel_    = this->get_parameter("normal_vel").as_double_array();
        normal_accel_  = this->get_parameter("normal_accel").as_double_array();
        normal_decel_  = this->get_parameter("normal_decel").as_double_array();

        fast_vel_    = this->get_parameter("fast_vel").as_double_array();
        fast_accel_  = this->get_parameter("fast_accel").as_double_array();
        fast_decel_  = this->get_parameter("fast_decel").as_double_array();

        // Khởi tạo ODrive interface
        odrive_ = std::make_shared<can_odrive_interface::ODriveInterface>();   

        // Subscribe topic /velocity_modes
        subscription_ = this->create_subscription<std_msgs::msg::String>(
            "/velocity_modes", 10,
            std::bind(&VelocityModesNode::velocity_callback, this, std::placeholders::_1)
        );

        RCLCPP_INFO(this->get_logger(), "VelocityModesNode started, waiting for /velocity_modes...");
    }

private:
    
    std::shared_ptr<can_odrive_interface::ODriveInterface> odrive_;

    rclcpp::Subscription<std_msgs::msg::String>::SharedPtr subscription_;

    std::vector<double> slow_vel_, slow_accel_, slow_decel_;
    std::vector<double> normal_vel_, normal_accel_, normal_decel_;
    std::vector<double> fast_vel_, fast_accel_, fast_decel_;

    void velocity_callback(const std_msgs::msg::String::SharedPtr msg) {
        std::string mode = msg->data;
        RCLCPP_INFO(this->get_logger(), "Received velocity mode: %s", mode.c_str());
        apply_velocity_mode(mode);
    }

    void apply_velocity_mode(const std::string& mode) {
        std::vector<double> vel, accel, decel;

        if (mode == "slow") {
            vel = slow_vel_; accel = slow_accel_; decel = slow_decel_;
        } else if (mode == "normal") {
            vel = normal_vel_; accel = normal_accel_; decel = normal_decel_;
        } else if (mode == "fast") {
            vel = fast_vel_; accel = fast_accel_; decel = fast_decel_;
        } else {
            RCLCPP_WARN(this->get_logger(), "Unknown mode %s", mode.c_str());
            return;
        }

        for (int i=0; i<3; ++i) {
            odrive_->SetTrajectoryVelLimit(i, vel[i]);
            odrive_->SetTrajectoryAccelLimit(i, accel[i], decel[i]);
        }
        RCLCPP_INFO(this->get_logger(), "Applied %s mode: vel[%f,%f,%f], accel[%f,%f,%f], decel[%f,%f,%f]",
                    mode.c_str(),
                    vel[0], vel[1], vel[2],
                    accel[0], accel[1], accel[2],
                    decel[0], decel[1], decel[2]);
    }
};

int main(int argc, char* argv[]) {
    rclcpp::init(argc, argv);
    rclcpp::spin(std::make_shared<VelocityModesNode>());
    rclcpp::shutdown();
    return 0;
}
