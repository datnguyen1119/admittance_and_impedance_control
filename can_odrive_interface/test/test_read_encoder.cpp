#include <rclcpp/rclcpp.hpp>
#include <can_odrive_interface/odrive_interface.hpp>
#include <std_msgs/msg/float32.hpp>

class TestReadEncoder : public rclcpp::Node {
public:
    TestReadEncoder() : Node("test_read_encoder") {
        odrive_ = std::make_shared<can_odrive_interface::ODriveInterface>();
        
        // Khởi tạo chế độ velocity cho Node 0 để đọc encoder
        odrive_->InitPositionMode(0);
        odrive_->InitPositionMode(1);

        
        // Bắt đầu đọc encoder
        odrive_->ReadEncoder();

        // Tạo subscription để nhận dữ liệu encoder
        pos_sub_0_ = this->create_subscription<std_msgs::msg::Float32>(
            "/motor/position", 10,
            std::bind(&TestReadEncoder::pos_callback_0, this, std::placeholders::_1));
        vel_sub_0_ = this->create_subscription<std_msgs::msg::Float32>(
            "/motor/velocity", 10,
            std::bind(&TestReadEncoder::vel_callback_0, this, std::placeholders::_1));

        RCLCPP_INFO(this->get_logger(), "Test Read Encoder Node started");
    }

private:
    void pos_callback_0(const std_msgs::msg::Float32::SharedPtr msg) {
        RCLCPP_INFO(this->get_logger(), "Node's Position: %.3f rad", msg->data);
    }

    void vel_callback_0(const std_msgs::msg::Float32::SharedPtr msg) {
        RCLCPP_INFO(this->get_logger(), "Node's Velocity: %.3f turns/s", msg->data);
    }

    std::shared_ptr<can_odrive_interface::ODriveInterface> odrive_;
    rclcpp::Subscription<std_msgs::msg::Float32>::SharedPtr pos_sub_0_;
    rclcpp::Subscription<std_msgs::msg::Float32>::SharedPtr vel_sub_0_;
};

int main(int argc, char *argv[]) {
    rclcpp::init(argc, argv);
    rclcpp::spin(std::make_shared<TestReadEncoder>());
    rclcpp::shutdown();
    return 0;
}