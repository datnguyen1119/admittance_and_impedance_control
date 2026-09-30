// #include <rclcpp/rclcpp.hpp>
// #include <can_odrive_interface/odrive_interface.hpp>

// class AdmittanceControl : public rclcpp::Node {
// public:
//     static constexpr double RAD_TO_REV = 4.0 / M_PI;
//     static constexpr double REV_TO_RAD = M_PI / 4.0;
//     AdmittanceControl() : Node("admittance_control") {
//         odrive_ = std::make_shared<can_odrive_interface::ODriveInterface>();
//         odrive_->InitPositionMode(2); 
//         odrive_->SetPosGain(2, 10);
//         odrive_->SendPositionCommand(2, 0.0,0,0);

//         RCLCPP_INFO(this->get_logger(), "Initialized motor node 2 in position mode.");

//         this->declare_parameter("M_admittance", 0.5);
//         this->declare_parameter("B_admittance", 1.5);
//         this->declare_parameter("K_admittance", 1.5);
//         // this->declare_parameter("M_admittance", 0.5);
//         // this->declare_parameter("B_admittance", 1.5);
//         // this->declare_parameter("K_admittance", 0.0);

//         // M_admittance_ = this->get_parameter("M_admittance").as_double();
//         // B_admittance_ = this->get_parameter("B_admittance").as_double();
//         // K_admittance_ = this->get_parameter("K_admittance").as_double();

//         odrive_->ReadEncoder(2); // Đọc
//         float initial_turn_ = 0.0f;
//         float initial_vel_ = 0.0f;
//         odrive_->get_encoder_data_nolog(2, initial_turn_, initial_vel_);
//         theta_meas_ = initial_turn_ * REV_TO_RAD; 

//         theta_a_ = theta_meas_;
//         theta_dot_a_ = 0.0;
//         theta_ddot_a_ = 0.0;

//         torque_external_ = 0.0;

//         timer_ = this->create_wall_timer(
//                 std::chrono::milliseconds(1),   // chu kỳ
//                 std::bind(&AdmittanceControl::timer_callback, this)
//         );
//     }
// private:
//     std::shared_ptr<can_odrive_interface::ODriveInterface> odrive_;
//     double M_admittance_;
//     double B_admittance_;
//     double K_admittance_;
    
//     // ===== STATE CỦA BỘ ADMITTANCE =====
//     double theta_a_;      // vị trí ảo
//     double theta_dot_a_;  // vận tốc ảo
//     double theta_ddot_a_; // gia tốc ảo
//     double theta_meas_;   // vị trí thực tế

//     double torque_external_;

//     rclcpp::TimerBase::SharedPtr timer_;

//     void timer_callback() {
//         const double dt = 0.001;

//         M_admittance_ = this->get_parameter("M_admittance").as_double();
//         B_admittance_ = this->get_parameter("B_admittance").as_double();
//         K_admittance_ = this->get_parameter("K_admittance").as_double();

//         odrive_->ReadTorque(2); // Đọc
//         float torque_external_ = 0.0f;
//         odrive_->get_torque_data_nolog(2, torque_external_);

//         odrive_->ReadEncoder(2); // Đọc
//         float turn_ = 0.0f;
//         float vel_ = 0.0f;
//         odrive_->get_encoder_data_nolog(2, turn_, vel_);
//         theta_meas_ = turn_ * REV_TO_RAD; // Cập nhật vị trí thực tế từ ODrive

//         theta_ddot_a_ = (1.0 / M_admittance_) * (torque_external_ - B_admittance_ * theta_dot_a_ - K_admittance_ * theta_a_);

//         theta_dot_a_ += theta_ddot_a_ * dt; // dt = 1ms

//         theta_a_ += theta_dot_a_ * dt;

//         double theta_cmd_ = theta_meas_ + theta_a_;
//         RCLCPP_INFO(this->get_logger(), "Torque ext: %.4f Nm, Theta_a: %.4f rad", torque_external_, theta_a_);
        
//         odrive_->SendPositionCommandRad(2, theta_cmd_, 0, 0);
//     }

// };

// int main(int argc, char** argv) {
//     rclcpp::init(argc, argv);
//     rclcpp::spin(std::make_shared<AdmittanceControl>());
//     rclcpp::shutdown();
//     return 0;
// }


//--------------------------------
#include <rclcpp/rclcpp.hpp>
#include <can_odrive_interface/odrive_interface.hpp>
#include <array>

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
            // odrive_->SetPosGain(i, 10);
            // odrive_->SendPositionCommand(i, 0.0, 0, 0); 
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
            float initial_vel = 0.0f;
            odrive_->get_encoder_data(i, initial_turn, initial_vel);
            
            theta_meas_[i] = initial_turn * REV_TO_RAD;
            theta_a_[i] = theta_meas_[i];
            theta_dot_a_[i] = 0.0;
            theta_ddot_a_[i] = 0.0;
            torque_external_[i] = 0.0;
        }

        // Timer 1ms
        timer_ = this->create_wall_timer(
            std::chrono::milliseconds(1),
            std::bind(&AdmittanceControl::timer_callback, this)
        );

        RCLCPP_INFO(this->get_logger(), "Admittance control initialized for 3 motors");
    }

private:
    std::shared_ptr<can_odrive_interface::ODriveInterface> odrive_;
    
    // Arrays cho 3 động cơ
    std::array<double, NUM_MOTORS> M_admittance_;
    std::array<double, NUM_MOTORS> B_admittance_;
    std::array<double, NUM_MOTORS> K_admittance_;
    
    std::array<double, NUM_MOTORS> theta_a_;        // vị trí ảo
    std::array<double, NUM_MOTORS> theta_dot_a_;    // vận tốc ảo
    std::array<double, NUM_MOTORS> theta_ddot_a_;   // gia tốc ảo
    std::array<double, NUM_MOTORS> theta_meas_;     // vị trí thực tế
    std::array<double, NUM_MOTORS> torque_external_;

    rclcpp::TimerBase::SharedPtr timer_;

    void timer_callback() {
        const double dt = 0.001; // 1ms

        // Xử lý từng động cơ
        for (int i = 0; i < NUM_MOTORS; i++) {
            // Đọc parameters
            std::string prefix = "motor_" + std::to_string(i) + "_";
            M_admittance_[i] = this->get_parameter(prefix + "M_admittance").as_double();
            B_admittance_[i] = this->get_parameter(prefix + "B_admittance").as_double();
            K_admittance_[i] = this->get_parameter(prefix + "K_admittance").as_double();

            // Đọc torque
            odrive_->ReadTorque(i);
            float torque = 0.0f;
            odrive_->get_torque_data(i, torque);
            torque_external_[i] = 3.75 * torque;

            // Đọc encoder
            odrive_->ReadEncoder(i);
            float turn = 0.0f;
            float vel = 0.0f;
            odrive_->get_encoder_data(i, turn, vel);
            theta_meas_[i] = turn * REV_TO_RAD;

            // Admittance control equation
            theta_ddot_a_[i] = (1.0 / M_admittance_[i]) * 
                               (torque_external_[i] - 
                                B_admittance_[i] * theta_dot_a_[i] - 
                                K_admittance_[i] * theta_a_[i]);

            // Tích phân
            theta_dot_a_[i] += theta_ddot_a_[i] * dt;
            theta_a_[i] += theta_dot_a_[i] * dt;

            // Command position
            double theta_cmd = theta_meas_[i] + theta_a_[i];
            
            // Gửi lệnh
            odrive_->SendPositionCommandRad(i, theta_cmd, 0, 0);

            // Log thông tin
            RCLCPP_INFO(this->get_logger(), 
                       "Motor %d - Torque: %.4f Nm, Theta_a: %.4f rad, Theta_cmd: %.4f rad", 
                       i, torque_external_[i], theta_a_[i], theta_cmd);
        }
    }
};

int main(int argc, char** argv) {
    rclcpp::init(argc, argv);
    rclcpp::spin(std::make_shared<AdmittanceControl>());
    rclcpp::shutdown();
    return 0;
}