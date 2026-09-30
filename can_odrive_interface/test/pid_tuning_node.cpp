// #include <rclcpp/rclcpp.hpp>
// #include <can_odrive_interface/odrive_interface.hpp>

// class PIDTuningNode : public rclcpp::Node {
// public:
//     PIDTuningNode() : Node("pid_tuning_node") {
        
//         joint_number_ = 3;
//         this->declare_parameter<std::vector<double>>("Kp", {0.0, 0.0, 0.0});
//         this->declare_parameter<std::vector<double>>("Ki", {0.0, 0.0, 0.0});
//         this->declare_parameter<std::vector<double>>("Kd", {0.0, 0.0, 0.0});

//         load_parameters();
//         parameter_callback_handle_ = this->add_on_set_parameters_callback(
//             std::bind(&PIDTuningNode::on_parameter_change, this, std::placeholders::_1)
//         );

//         odrive_ = std::make_shared<can_odrive_interface::ODriveInterface>();

// private:
//         int joint_number_;
//         std::vector<double> Kp_, Ki_, Kd_;
//         std::shared_ptr<can_odrive_interface::ODriveInterface> odrive_;
//         OnSetParametersCallbackHandle::SharedPtr parameter_callback_handle_;

//         void load_parameters() {
//             Kp_ = this->get_parameter("Kp").as_double_array();
//             Ki_ = this->get_parameter("Ki").as_double_array();
//             Kd_ = this->get_parameter("Kd").as_double_array();

//             if (Kp_.size() != joint_number_ || Ki_.size() != joint_number_ || Kd_.size() != joint_number_) {
//                 RCLCPP_ERROR(this->get_logger(), "Parameter size mismatch with joint_number (%d)", joint_number_);
//                 throw std::runtime_error("Invalid parameter size");
//             }

//             RCLCPP_INFO(this->get_logger(), "Loaded parameters: Kp: [%s], Ki: [%s], Kd: [%s]",
//                         vector_to_string(Kp_).c_str(),
//                         vector_to_string(Ki_).c_str(),
//                         vector_to_string(Kd_).c_str());
//         }
//         std::string vector_to_string(const std::vector<double>& vec) {
//             std::string result;
//             for (size_t i = 0; i < vec.size(); ++i) {
//                 result += std::to_string(vec[i]);
//                 if (i < vec.size() - 1) {
//                     result += ", ";
//                 }
//             }
//             return result;
//         }
//         void update_motor_pids(){
//             for (int i = 0; i < joint_number_; ++i) {
//                 odrive_->SetPIDGains(i, Kp_[i], Ki_[i], Kd_[i]);
//                 RCLCPP_INFO(this->get_logger(), "Updated motor %d PID gains to Kp=%.2f, Ki=%.2f, Kd=%.2f",
//                             i, Kp_[i], Ki_[i], Kd_[i]);
//             }
//         }

#include <rclcpp/rclcpp.hpp>
#include <can_odrive_interface/odrive_interface.hpp>
#include <chrono>
#include <thread>

class PIDTuningNode : public rclcpp::Node {
public:
    PIDTuningNode() : Node("pid_tuning_node") {
        // Khai báo parameter: [node_id, Kp, Ki, Kd]
        this->declare_parameter<std::vector<double>>("pid_gain", {0.0, 0.0, 0.0, 0.0});

        odrive_ = std::make_shared<can_odrive_interface::ODriveInterface>();

        // Callback khi parameter thay đổi
        parameter_callback_handle_ = this->add_on_set_parameters_callback(
            std::bind(&PIDTuningNode::on_parameter_change, this, std::placeholders::_1)
        );

        RCLCPP_INFO(this->get_logger(), "PID Tuning Node started. Use parameter 'pid_gain' = [id, Kp, Ki, Kd]");
    }

private:
    std::shared_ptr<can_odrive_interface::ODriveInterface> odrive_;
    OnSetParametersCallbackHandle::SharedPtr parameter_callback_handle_;

    rcl_interfaces::msg::SetParametersResult on_parameter_change(const std::vector<rclcpp::Parameter> &params) {
        rcl_interfaces::msg::SetParametersResult result;
        result.successful = true;
        result.reason = "PID update processed";

        for (const auto &param : params) {
            if (param.get_name() == "pid_gain") {
                auto values = param.as_double_array();

                if (values.size() != 4) {
                    RCLCPP_ERROR(this->get_logger(), "Invalid format. Expected [id, Kp, Ki, Kd]");
                    result.successful = false;
                    result.reason = "Invalid parameter size";
                    return result;
                }

                int node_id = static_cast<int>(values[0]);
                double kp = values[1];
                double ki = values[2];
                double kd = values[3];

                RCLCPP_INFO(this->get_logger(),
                            "Setting PID for motor ID %d -> Kp=%.3f, Ki=%.3f, Kd=%.3f",
                            node_id, kp, ki, kd);

                odrive_->SetPIDGains(node_id, kp, ki, kd);
                RCLCPP_INFO(this->get_logger(), "Waiting 2s before saving configuration...");
                std::this_thread::sleep_for(std::chrono::seconds(2));
                odrive_->SaveConfiguration(node_id);
                RCLCPP_INFO(this->get_logger(), "Rebooting motor ID %d...", node_id);
                odrive_->Reboot(node_id);
                RCLCPP_INFO(this->get_logger(), "Motor ID %d updated and rebooted.", node_id);
            }
        }
        return result;
    }
};

int main(int argc, char *argv[]) {
    rclcpp::init(argc, argv);
    rclcpp::spin(std::make_shared<PIDTuningNode>());
    rclcpp::shutdown();
    return 0;
}
