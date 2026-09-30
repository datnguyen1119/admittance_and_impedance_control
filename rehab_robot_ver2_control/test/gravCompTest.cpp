// #include <rclcpp/rclcpp.hpp>
// #include <can_odrive_interface/odrive_interface.hpp>
// #include <std_msgs/msg/float32.hpp>
// #include <cmath>

// const double REV_TO_RAD = M_PI / 4.0;   // 1 turn = π/4 rad

// class ODriveControlNode : public rclcpp::Node {
// public:
//     ODriveControlNode() : Node("odrive_control_node")
//     {
//         // ------------------ 1. Khởi tạo ODrive ------------------
//         odrive_ = std::make_shared<can_odrive_interface::ODriveInterface>();
//         // odrive_->InitPositionMode(0);
//         // odrive_->InitPositionMode(1);
//         odrive_->InitPositionMode(2);

//         // ------------------ 2. Subscribed Commands ------------------
//         position_sub_ = this->create_subscription<std_msgs::msg::Float32>(
//             "/odrive/target_position", 10,
//             std::bind(&ODriveControlNode::position_callback, this, std::placeholders::_1));

//         torque_sub_ = this->create_subscription<std_msgs::msg::Float32>(
//             "/odrive/torque_ff", 10,
//             std::bind(&ODriveControlNode::torque_callback, this, std::placeholders::_1));

//         // ------------------ 3. Timer gửi lệnh 100 Hz ------------------
//         timer_cmd_ = this->create_wall_timer(
//             std::chrono::milliseconds(10),
//             std::bind(&ODriveControlNode::send_command_loop, this));

//         RCLCPP_INFO(this->get_logger(), "ODrive Control Node started");
//     }

// private:

//     // ------------------------- CALLBACK -------------------------
//     void position_callback(const std_msgs::msg::Float32::SharedPtr msg)
//     {
//         target_pos_rad_ = msg->data;
//         RCLCPP_INFO(this->get_logger(), "Nhận target position: %.4f rad", target_pos_rad_);
//     }

//     void torque_callback(const std_msgs::msg::Float32::SharedPtr msg)
//     {
//         target_torque_nm_ = msg->data;
//         RCLCPP_INFO(this->get_logger(), "Nhận torque_ff: %.4f Nm", target_torque_nm_);
//     }

//     // ------------------------- SEND CMD LOOP -------------------------
//     void send_command_loop()
//     {
//         // 1) Convert position rad → turns
//         float pos_turns = target_pos_rad_ / REV_TO_RAD;

//         // 2) Convert torque Nm → raw torque_ff (Nm * 1000)
//         int16_t torque_raw = static_cast<int16_t>(target_torque_nm_ * 1000.0f);

//         // 3) Gửi CAN cho từng động cơ
//         // odrive_->SendPositionCommand(0, pos_turns, 0, torque_raw);
//         // odrive_->SendPositionCommand(1, pos_turns, 0, torque_raw);
//         odrive_->SendPositionCommand(2, pos_turns, 0, torque_raw);

//         RCLCPP_INFO(this->get_logger(),
//             "SEND → pos: %.3f rad (%.3f turns), torque_ff: %.3f Nm (%d raw)",
//             target_pos_rad_, pos_turns, target_torque_nm_, torque_raw
//         );
//     }

//     // ------------------------- VARIABLES -------------------------
//     std::shared_ptr<can_odrive_interface::ODriveInterface> odrive_;

//     rclcpp::Subscription<std_msgs::msg::Float32>::SharedPtr position_sub_;
//     rclcpp::Subscription<std_msgs::msg::Float32>::SharedPtr torque_sub_;
//     rclcpp::TimerBase::SharedPtr timer_cmd_;

//     float target_pos_rad_ = 0.0;
//     float target_torque_nm_ = 0.0;
// };

// int main(int argc, char *argv[])
// {
//     rclcpp::init(argc, argv);
//     rclcpp::spin(std::make_shared<ODriveControlNode>());
//     rclcpp::shutdown();
//     return 0;
// }

// #include <rclcpp/rclcpp.hpp>
// #include <std_msgs/msg/float32.hpp>
// #include <can_odrive_interface/odrive_interface.hpp>
// #include <rehab_robot_ver2_control/gravityCompNode.hpp>


// const double REV_TO_RAD = M_PI / 4.0;   // 1 turn = π/4 rad

// class ODriveGravityNode : public rclcpp::Node {
// public:
//     ODriveGravityNode()
//         : Node("odrive_gravity_node"),
//           gc_("/home/dat/Workspace/gravitational_compensation/src/rehab_robot_ver2/urdf/rehab_robot_ver2.urdf")   // TODO: đổi path đúng
//     {
//         // ------------------ 1. Khởi tạo ODrive ------------------
//         odrive_ = std::make_shared<can_odrive_interface::ODriveInterface>();
//         odrive_->InitPositionMode(2);   // Motor node 2
//         odrive_->SendPositionCommand(2, 0.0,0,0); // Khởi động về 0 turns
//         odrive_->SetPosGain(2, 5.0f); // Tăng Pos Gain để bù trọng lực tốt hơn

//         // ------------------ 2. Nhận vị trí mong muốn từ topic ------------------
//         pos_sub_ = this->create_subscription<std_msgs::msg::Float32>(
//             "/odrive/target_position", 10,
//             std::bind(&ODriveGravityNode::pos_callback, this, std::placeholders::_1));


//         timer_ = this->create_wall_timer(
//             std::chrono::milliseconds(10),
//             std::bind(&ODriveGravityNode::control_loop, this));

//         RCLCPP_INFO(this->get_logger(), "ODrive Gravity Compensation Node started");
//     }

// private:

//     // ------------------------- CALLBACK nhận vị trí -------------------------
//     void pos_callback(const std_msgs::msg::Float32::SharedPtr msg)
//     {
//         target_pos_rad_ = msg->data;
//         RCLCPP_INFO(this->get_logger(),
//             "Nhận target position: %.4f rad", target_pos_rad_);
//     }

//     // ------------------------- VÒNG LẶP CHÍNH -------------------------
//     void control_loop()
//     {
//         // (1) Đọc encoder từ ODrive
//         odrive_->ReadEncoder();

//         // (2) Cập nhật q vào GravityComp
//         gc_.UpdateFromOdrive(*odrive_);

//         // (3) Tính torque bù trọng lực bằng RNEA
//         gc_.ComputeGravityCompensation();

//         // (4) Lấy torque của joint index = 2 (motor node 2)
//         double tau_nm = gc_.GetGravityTorque(2);   // Nm

//         // (5) Scale sang raw torque
//         int16_t torque_raw = static_cast<int16_t>(tau_nm * 1000.0);

//         // (6) Convert position rad → turns
//         float pos_turns = target_pos_rad_ / REV_TO_RAD;

//         // (7) Gửi lệnh tới motor NodeID = 2
//         // odrive_->SendPositionCommand(2, pos_turns, 0, torque_raw);
//         odrive_->SendPositionCommand(2, pos_turns, 0, -5000);

//         RCLCPP_INFO(this->get_logger(),
//             "SEND → pos: %.3f rad (%.3f turns) | tau: %.3f Nm (%d raw)",
//             target_pos_rad_, pos_turns, tau_nm, torque_raw
//         );
//     }

//     // ----------------------------- VARIABLES -----------------------------
//     std::shared_ptr<can_odrive_interface::ODriveInterface> odrive_;
//     ArmGravityComp gc_;

//     rclcpp::Subscription<std_msgs::msg::Float32>::SharedPtr pos_sub_;
//     rclcpp::TimerBase::SharedPtr timer_;

//     float target_pos_rad_ = 0.0;
// };

// int main(int argc, char *argv[])
// {
//     rclcpp::init(argc, argv);
//     rclcpp::spin(std::make_shared<ODriveGravityNode>());
//     rclcpp::shutdown();
//     return 0;
// }

#include <rclcpp/rclcpp.hpp>
#include <std_msgs/msg/float32.hpp>
#include <can_odrive_interface/odrive_interface.hpp>
#include <rehab_robot_ver2_control/gravityCompNode.hpp>

const double REV_TO_RAD = M_PI / 4.0;   // chú ý: phải khớp với cách bạn định nghĩa "turn"

class ODriveGravityNode : public rclcpp::Node {
public:
    ODriveGravityNode()
        : Node("odrive_gravity_node"),
          gc_("/home/dat/Workspace/gravitational_compensation/src/rehab_robot_ver2/urdf/rehab_robot_ver2.urdf")
    {
        odrive_ = std::make_shared<can_odrive_interface::ODriveInterface>();
        odrive_->InitPositionMode(2);   // Motor node 2

        // Option: đưa về 0
        odrive_->SendPositionCommand(2, 0.0f, 0, 0);

        // Gain tùy bạn (đừng để quá cao lúc test torque_ff để tránh "đánh nhau")
        odrive_->SetPosGain(2, 5.0f);

        // Sub target position (rad)
        pos_sub_ = this->create_subscription<std_msgs::msg::Float32>(
            "/odrive/target_position", 10,
            std::bind(&ODriveGravityNode::pos_callback, this, std::placeholders::_1));

        // Sub torque_ff (Nm)
        torque_sub_ = this->create_subscription<std_msgs::msg::Float32>(
            "/odrive/torque_ff", 10,
            std::bind(&ODriveGravityNode::torque_callback, this, std::placeholders::_1));

        timer_ = this->create_wall_timer(
            std::chrono::milliseconds(10),  // 100Hz
            std::bind(&ODriveGravityNode::control_loop, this));

        RCLCPP_INFO(this->get_logger(), "ODrive node started (pos + custom torque_ff)");
    }

private:
    static int16_t clamp_i16(int v) {
        if (v > 32767) return 32767;
        if (v < -32768) return -32768;
        return static_cast<int16_t>(v);
    }

    void pos_callback(const std_msgs::msg::Float32::SharedPtr msg)
    {
        target_pos_rad_ = msg->data;
        RCLCPP_INFO(this->get_logger(), "Target pos = %.4f rad", target_pos_rad_);
    }

    void torque_callback(const std_msgs::msg::Float32::SharedPtr msg)
    {
        torque_ff_nm_ = msg->data; // Nm
        RCLCPP_INFO(this->get_logger(), "Torque_ff = %.4f Nm", torque_ff_nm_);
    }

    void control_loop()
    {
        // 1) Read encoder (nếu bạn cần)
        odrive_->ReadEncoder();

        // 2) Convert position rad -> turns
        const float pos_turns = static_cast<float>(target_pos_rad_ / REV_TO_RAD);

        // 3) Convert torque_ff (Nm) -> raw int16 (0.001 Nm/LSB)
        const int torque_raw_i32 = static_cast<int>(std::lround(torque_ff_nm_ * 1000.0f));
        const int16_t torque_raw = clamp_i16(torque_raw_i32);

        // 4) Send command continuously
        odrive_->SendPositionCommand(2, pos_turns, 0, torque_raw);

        // 5) Throttled log (1 lần/giây)
        RCLCPP_INFO_THROTTLE(
            this->get_logger(),
            *this->get_clock(),
            1000,
            "SEND node=2 | pos=%.3f rad (%.3f turns) | torque_ff=%.3f Nm (raw=%d)",
            target_pos_rad_, pos_turns, torque_ff_nm_, (int)torque_raw
        );
    }

    std::shared_ptr<can_odrive_interface::ODriveInterface> odrive_;
    ArmGravityComp gc_;

    rclcpp::Subscription<std_msgs::msg::Float32>::SharedPtr pos_sub_;
    rclcpp::Subscription<std_msgs::msg::Float32>::SharedPtr torque_sub_;
    rclcpp::TimerBase::SharedPtr timer_;

    float target_pos_rad_ = 0.0f;
    float torque_ff_nm_ = 0.0f;
};

int main(int argc, char *argv[])
{
    rclcpp::init(argc, argv);
    rclcpp::spin(std::make_shared<ODriveGravityNode>());
    rclcpp::shutdown();
    return 0;
}
