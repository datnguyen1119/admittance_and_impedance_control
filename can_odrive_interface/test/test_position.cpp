#include <rclcpp/rclcpp.hpp>
#include <can_odrive_interface/odrive_interface.hpp>
#include <std_msgs/msg/float32.hpp>
#include <cmath>

const double REV_TO_RAD = M_PI / 4.0; // 1 turn = π/4 radian

class TestPosition : public rclcpp::Node {
public:
    TestPosition() : Node("test_position"), desired_position_rad_(0.0), position_received_(false) {
        odrive_ = std::make_shared<can_odrive_interface::ODriveInterface>();
        
        // Khởi tạo chế độ position cho Node 0
        odrive_->InitPositionMode(1);
        
        // Tạo subscriber cho topic /desired_position
        position_sub_ = this->create_subscription<std_msgs::msg::Float32>(
            "/desired_position", 10,
            std::bind(&TestPosition::position_callback, this, std::placeholders::_1));
        
        encoder_pub_0_ = this->create_publisher<std_msgs::msg::Float32>(
            "/motor0/position", 10);

        odrive_->SendPositionCommand(1, 0.0,0,0);
        
        // Tạo timer để gửi lệnh vị trí mỗi 2 giây
        timer_send_pos_ = this->create_wall_timer(
            std::chrono::seconds(2),
            std::bind(&TestPosition::send_position, this));

        timer_read_encoder_ = this->create_wall_timer(
            std::chrono::milliseconds(10),
            std::bind(&TestPosition::read_encoder, this));
        
        RCLCPP_INFO(this->get_logger(), "Test Position Node started");
    }

private:
    void position_callback(const std_msgs::msg::Float32::SharedPtr msg) {
        desired_position_rad_ = msg->data;
        position_received_ = true;
        RCLCPP_INFO(this->get_logger(), "Nhận vị trí mong muốn: %.2f rad", desired_position_rad_);
    }

    // OdriveComms comms_;
    // void send_position() {
    //     if (!position_received_) return;
    //     comms_.cmd = desired_position_rad_;
    //     odrive_->SendPositionCommand(comms_.node_id, comms_.convert_command_to_turns(), 
    //                                 comms_.velocity_ff, comms_.torque_ff);
    // }
    void send_position() {
        if (!position_received_) {
            RCLCPP_WARN(this->get_logger(), "Chưa nhận vị trí từ topic /desired_position, không gửi lệnh");
            return;
        }

        float position_turns = desired_position_rad_ / REV_TO_RAD; // Chuyển radian sang turns
        odrive_->SendPositionCommand(1, position_turns,0,0);
        // odrive_->SendPositionCommand(1, 0.0,0,0);
        RCLCPP_INFO(this->get_logger(), "Gửi vị trí %.2f rad (%.2f turns) cho Node 0", 
                    desired_position_rad_, position_turns);
    }
    void read_encoder() {
        // Cập nhật dữ liệu encoder từ ODrive
        odrive_->ReadEncoder();

        // Motor 0
        float pos_turns0 = 0.0f, vel_turns0 = 0.0f;
        odrive_->get_encoder_data(2, pos_turns0, vel_turns0);
        float pos_rad0 = pos_turns0 * REV_TO_RAD;

        auto msg0 = std_msgs::msg::Float32();
        msg0.data = pos_rad0;
        encoder_pub_0_->publish(msg0);
        RCLCPP_INFO(this->get_logger(), "Node 0 - Encoder: %.2f turns (%.2f rad)", pos_turns0, pos_rad0);
    }

    std::shared_ptr<can_odrive_interface::ODriveInterface> odrive_;
    rclcpp::Subscription<std_msgs::msg::Float32>::SharedPtr position_sub_;
    rclcpp::Publisher<std_msgs::msg::Float32>::SharedPtr encoder_pub_0_;
    rclcpp::TimerBase::SharedPtr timer_send_pos_;
    rclcpp::TimerBase::SharedPtr timer_read_encoder_;
    float desired_position_rad_;
    bool position_received_;
};

int main(int argc, char *argv[]) {
    rclcpp::init(argc, argv);
    rclcpp::spin(std::make_shared<TestPosition>());
    rclcpp::shutdown();
    return 0;
}


// *Encoder la tuyet doi khong thay doi khi tat vi tri encoder giu nguyen khong thay doi