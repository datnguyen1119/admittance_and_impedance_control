

#include <rclcpp/rclcpp.hpp>
#include <std_msgs/msg/float32.hpp>
#include <can_odrive_interface/odrive_interface.hpp>

class SimpleVelocityNode : public rclcpp::Node {
public:
    SimpleVelocityNode()
        : Node("simple_velocity_node")
    {
        odrive_ = std::make_shared<can_odrive_interface::ODriveInterface>();
        odrive_->InitVelocityMode(0);   // Motor node 0
        odrive_->InitVelocityMode(1);   // Motor node 1
        odrive_->InitVelocityMode(2);   // Motor node 2

        // Declare parameters for target velocities
        this->declare_parameter<float>("target_velocity_0", 0.0f);
        this->declare_parameter<float>("target_velocity_1", 0.0f);
        this->declare_parameter<float>("target_velocity_2", 0.0f);

        // Add parameter callback
        param_callback_handle_ = this->add_on_set_parameters_callback(
            std::bind(&SimpleVelocityNode::on_parameter_change, this, std::placeholders::_1));

        RCLCPP_INFO(this->get_logger(), "Simple velocity node started - use params 'target_velocity_0', 'target_velocity_1', 'target_velocity_2' to control");
    }

private:
    rcl_interfaces::msg::SetParametersResult on_parameter_change(
        const std::vector<rclcpp::Parameter> &parameters)
    {
        rcl_interfaces::msg::SetParametersResult result;
        result.successful = true;
        result.reason = "success";

        for (const auto &param : parameters) {
            if (param.get_name() == "target_velocity_0") {
                float target_vel = param.as_double();
                odrive_->SendVelocityCommand(0, target_vel);
                RCLCPP_INFO(this->get_logger(), "Motor 0: Target velocity = %.4f turns/sec", target_vel);
            } else if (param.get_name() == "target_velocity_1") {
                float target_vel = param.as_double();
                odrive_->SendVelocityCommand(1, target_vel);
                RCLCPP_INFO(this->get_logger(), "Motor 1: Target velocity = %.4f turns/sec", target_vel);
            } else if (param.get_name() == "target_velocity_2") {
                float target_vel = param.as_double();
                odrive_->SendVelocityCommand(2, target_vel);
                RCLCPP_INFO(this->get_logger(), "Motor 2: Target velocity = %.4f turns/sec", target_vel);
            }
        }

        return result;
    }

    std::shared_ptr<can_odrive_interface::ODriveInterface> odrive_;
    rclcpp::node_interfaces::OnSetParametersCallbackHandle::SharedPtr param_callback_handle_;
};

int main(int argc, char *argv[])
{
    rclcpp::init(argc, argv);
    rclcpp::spin(std::make_shared<SimpleVelocityNode>());
    rclcpp::shutdown();
    return 0;
}
