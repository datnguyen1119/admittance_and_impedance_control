#include <rclcpp/rclcpp.hpp>
#include <can_odrive_interface/odrive_interface.hpp>
#include <array>
#include <string>
#include <chrono>

class AdmittanceControl : public rclcpp::Node {
public:
    static constexpr double RAD_TO_REV = 4.0 / M_PI;
    static constexpr double REV_TO_RAD = M_PI / 4.0;
    static constexpr int NUM_MOTORS = 3;

    AdmittanceControl() : Node("admittance_control") {
        odrive_ = std::make_shared<can_odrive_interface::ODriveInterface>();

        // Khởi tạo 3 động cơ (node 0, 1, 2)
        for (int i = 0; i < NUM_MOTORS; i++) {
            odrive_->InitPositionMode(i);
            RCLCPP_INFO(this->get_logger(), "Initialized motor node %d in position mode.", i);
        }

        // Khai báo parameters cho từng động cơ
        for (int i = 0; i < NUM_MOTORS; i++) {
            std::string prefix = "motor_" + std::to_string(i) + "_";
            this->declare_parameter(prefix + "M_admittance", 0.5);
            this->declare_parameter(prefix + "B_admittance", 1.5);
            this->declare_parameter(prefix + "K_admittance", 1.5);
        }

        // Đọc vị trí ban đầu cho cả 3 động cơ
        for (int i = 0; i < NUM_MOTORS; i++) {
            odrive_->ReadEncoder(i);

            float initial_turn = 0.0f;
            float initial_vel  = 0.0f;
            odrive_->get_encoder_data(i, initial_turn, initial_vel);

            theta_meas_[i]      = initial_turn * REV_TO_RAD;

            // NOTE: Bạn đang dùng theta_cmd = theta_meas + theta_a
            // Nếu theta_a là "offset" thì nên set theta_a=0.
            // Ở đây mình giữ nguyên y như code gốc của bạn:
            theta_a_[i]         = theta_meas_[i];
            theta_dot_a_[i]     = 0.0;
            theta_ddot_a_[i]    = 0.0;
            torque_external_[i] = 0.0;
        }

        // Timer 1ms: SendPos 1000Hz, Polling ~400Hz (trung bình)
        timer_ = this->create_wall_timer(
            std::chrono::milliseconds(1),
            std::bind(&AdmittanceControl::timer_callback, this)
        );

        RCLCPP_INFO(this->get_logger(), "Admittance control initialized for 3 motors (poll ~400Hz, sendpos 1000Hz)");
    }

private:
    std::shared_ptr<can_odrive_interface::ODriveInterface> odrive_;

    std::array<double, NUM_MOTORS> M_admittance_{};
    std::array<double, NUM_MOTORS> B_admittance_{};
    std::array<double, NUM_MOTORS> K_admittance_{};

    std::array<double, NUM_MOTORS> theta_a_{};        // vị trí ảo
    std::array<double, NUM_MOTORS> theta_dot_a_{};    // vận tốc ảo
    std::array<double, NUM_MOTORS> theta_ddot_a_{};   // gia tốc ảo
    std::array<double, NUM_MOTORS> theta_meas_{};     // vị trí thực tế
    std::array<double, NUM_MOTORS> torque_external_{};

    rclcpp::TimerBase::SharedPtr timer_;

    int tick_ = 0;     // đếm tick 1ms

    // Polling ~400Hz bằng nhịp 2ms-3ms-2ms-3ms... (trung bình 2.5ms)
    bool should_poll_400hz() {
        // tick_ tăng mỗi 1ms
        // Poll ở tick % 5 == 2 và tick % 5 == 0:
        // tick: 2,5,7,10,12,15,... => khoảng cách 2,3,2,3,... ms
        const int m = tick_ % 5;
        return (m == 2 || m == 0);
    }

    void timer_callback() {
        const double dt = 0.001; // 1ms
        tick_++;

        const bool do_poll = should_poll_400hz();

        for (int i = 0; i < NUM_MOTORS; i++) {
            // Đọc parameters (tối ưu hơn: cache bằng parameter callback)
            std::string prefix = "motor_" + std::to_string(i) + "_";
            M_admittance_[i] = this->get_parameter(prefix + "M_admittance").as_double();
            B_admittance_[i] = this->get_parameter(prefix + "B_admittance").as_double();
            K_admittance_[i] = this->get_parameter(prefix + "K_admittance").as_double();

            // ===== Polling giảm xuống ~400Hz =====
            if (do_poll) {
                odrive_->ReadTorque(i);
                odrive_->ReadEncoder(i);
            }

            // Lấy data mới nhất (có thể là data vừa update hoặc cũ 1-2ms)
            float torque = 0.0f;
            odrive_->get_torque_data(i, torque);
            torque_external_[i] = 3.75 * torque;

            float turn = 0.0f;
            float vel  = 0.0f;
            odrive_->get_encoder_data(i, turn, vel);
            theta_meas_[i] = turn * REV_TO_RAD;

            // ===== Admittance update 1000Hz =====
            theta_ddot_a_[i] =
                (1.0 / M_admittance_[i]) *
                (torque_external_[i]
                 - B_admittance_[i] * theta_dot_a_[i]
                 - K_admittance_[i] * theta_a_[i]);

            theta_dot_a_[i] += theta_ddot_a_[i] * dt;
            theta_a_[i]     += theta_dot_a_[i] * dt;

            // ===== SendPos vẫn 1000Hz =====
            const double theta_cmd = theta_meas_[i] + theta_a_[i];
            odrive_->SendPositionCommandRad(i, static_cast<float>(theta_cmd), 0, 0);
        }

        // ===== Log chậm lại để không phá timing (20Hz) =====
        static int log_cnt = 0;
        if (++log_cnt % 50 == 0) { // 50ms ~ 20Hz
            RCLCPP_INFO(this->get_logger(),
                        "poll=%d | m0 tau=%.3f cmd=%.3f | m1 tau=%.3f cmd=%.3f | m2 tau=%.3f cmd=%.3f",
                        do_poll ? 1 : 0,
                        torque_external_[0], (theta_meas_[0] + theta_a_[0]),
                        torque_external_[1], (theta_meas_[1] + theta_a_[1]),
                        torque_external_[2], (theta_meas_[2] + theta_a_[2]));
        }
    }
};

int main(int argc, char** argv) {
    rclcpp::init(argc, argv);
    rclcpp::spin(std::make_shared<AdmittanceControl>());
    rclcpp::shutdown();
    return 0;
}
