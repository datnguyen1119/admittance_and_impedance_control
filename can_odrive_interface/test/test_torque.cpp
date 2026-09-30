#include <rclcpp/rclcpp.hpp>
#include <can_odrive_interface/odrive_interface.hpp>
#include <std_msgs/msg/float32.hpp>

class TestTorque : public rclcpp::Node {
public:
    TestTorque() : Node("test_torque") {
        odrive_ = std::make_shared<can_odrive_interface::ODriveInterface>();
        
        // Khởi tạo chế độ torque cho Node 0
        odrive_->InitTorqueMode(1);

        // Tạo timer để gửi lệnh torque mỗi 1 giây
        timer_ = this->create_wall_timer(
            std::chrono::seconds(1),
            std::bind(&TestTorque::send_torque, this));
        
        RCLCPP_INFO(this->get_logger(), "Test Torque Node started");
    }

private:
    void send_torque() {
        static float torque = 0.1; // Mô-men xoắn ban đầu: 0.1 Nm
        odrive_->SendTorqueCommand(0, torque);
        torque = -torque; // Đổi hướng mỗi lần gửi
    }

    std::shared_ptr<can_odrive_interface::ODriveInterface> odrive_;
    rclcpp::TimerBase::SharedPtr timer_;
};

int main(int argc, char *argv[]) {
    rclcpp::init(argc, argv);
    rclcpp::spin(std::make_shared<TestTorque>());
    rclcpp::shutdown();
    return 0;
}