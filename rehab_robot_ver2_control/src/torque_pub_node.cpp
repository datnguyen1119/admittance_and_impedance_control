#include <rclcpp/rclcpp.hpp>
#include <can_odrive_interface/odrive_interface.hpp>
#include <std_msgs/msg/float32.hpp>

class TorqueControlNode : public rclcpp::Node {
public:
    TorqueControlNode() : Node("torque_control_node") {
        odrive_ = std::make_shared<can_odrive_interface::ODriveInterface>();

        // Chuyển động cơ ID 0 sang chế độ torque control
        odrive_->InitTorqueMode(0);
        RCLCPP_INFO(this->get_logger(), "Initialized ODrive in Torque Control Mode for Node 0");

        // Subscriber để nhận giá trị torque mong muốn (Nm)
        torque_sub_ = this->create_subscription<std_msgs::msg::Float32>(
            "/motor/torque_cmd", 10,
            std::bind(&TorqueControlNode::torque_callback, this, std::placeholders::_1));

        // Có thể đọc phản hồi encoder để debug
        odrive_->ReadEncoder();

        RCLCPP_INFO(this->get_logger(), "Torque Control Node started");
    }

private:
    void torque_callback(const std_msgs::msg::Float32::SharedPtr msg) {
        float torque = msg->data;

        // Gửi torque tới động cơ ID 0
        odrive_->SendTorqueCommand(0, torque);

        RCLCPP_INFO(this->get_logger(), "Sent Torque Command: %.3f Nm", torque);
    }

    std::shared_ptr<can_odrive_interface::ODriveInterface> odrive_;
    rclcpp::Subscription<std_msgs::msg::Float32>::SharedPtr torque_sub_;
};

int main(int argc, char *argv[]) {
    rclcpp::init(argc, argv);
    rclcpp::spin(std::make_shared<TorqueControlNode>());
    rclcpp::shutdown();
    return 0;
}
