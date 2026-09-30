#include <rclcpp/rclcpp.hpp>
#include <can_odrive_interface/odrive_interface.hpp>
#include <chrono>
#include <thread>
#include <vector>

class TrajectoryTuningNode : public rclcpp::Node {
public:
    TrajectoryTuningNode() : Node("trajectory_tuning_node")
    {
        // ===== Declare ROS2 Parameters =====
        this->declare_parameter<std::vector<double>>("traj_vel_limit", {0.0, 0.0});
        this->declare_parameter<std::vector<double>>("traj_accel_limit", {0.0, 0.0, 0.0});
        this->declare_parameter<std::vector<double>>("traj_inertia", {0.0, 0.0});

        // ===== Init ODrive interface =====
        odrive_ = std::make_shared<can_odrive_interface::ODriveInterface>();

        // ===== Register parameter callback =====
        parameter_callback_handle_ = this->add_on_set_parameters_callback(
            std::bind(&TrajectoryTuningNode::on_parameter_change, this, std::placeholders::_1)
        );

        // ===== Info =====
        RCLCPP_INFO(this->get_logger(), "Trajectory Tuning Node started.");
        RCLCPP_INFO(this->get_logger(), "Use parameters:");
        RCLCPP_INFO(this->get_logger(), " - traj_vel_limit   = [id, vel_limit]");
        RCLCPP_INFO(this->get_logger(), " - traj_accel_limit = [id, accel_limit, decel_limit]");
        RCLCPP_INFO(this->get_logger(), " - traj_inertia     = [id, inertia]");
    }

private:
    std::shared_ptr<can_odrive_interface::ODriveInterface> odrive_;
    OnSetParametersCallbackHandle::SharedPtr parameter_callback_handle_;

    rcl_interfaces::msg::SetParametersResult
    on_parameter_change(const std::vector<rclcpp::Parameter> &params)
    {
        rcl_interfaces::msg::SetParametersResult result;
        result.successful = true;
        result.reason = "Trajectory parameter update processed";

        for (const auto &param : params) {
            const std::string &name = param.get_name();

            // =============================
            // 1. velocity limit (id, vel)
            // =============================
            if (name == "traj_vel_limit") {
                auto values = param.as_double_array();
                if (values.size() != 2) {
                    RCLCPP_ERROR(this->get_logger(), "traj_vel_limit expects [id, vel_limit]");
                    result.successful = false;
                    continue;
                }

                int node_id = static_cast<int>(values[0]);
                float vel = static_cast<float>(values[1]);

                odrive_->SetTrajectoryVelLimit(node_id, vel);
                RCLCPP_INFO(this->get_logger(),
                            "Updated vel_limit: motor %d -> %.3f", node_id, vel);
            }

            // ======================================================
            // 2. accel limit (id, accel, decel)
            // ======================================================
            else if (name == "traj_accel_limit") {
                auto values = param.as_double_array();
                if (values.size() != 3) {
                    RCLCPP_ERROR(this->get_logger(),
                                 "traj_accel_limit expects [id, accel_limit, decel_limit]");
                    result.successful = false;
                    continue;
                }

                int node_id = static_cast<int>(values[0]);
                float accel = static_cast<float>(values[1]);
                float decel = static_cast<float>(values[2]);

                odrive_->SetTrajectoryAccelLimit(node_id, accel, decel);
                RCLCPP_INFO(this->get_logger(),
                            "Updated accel/decel: motor %d -> (%.3f, %.3f)",
                            node_id, accel, decel);
            }

            // =============================
            // 3. inertia (id, inertia)
            // =============================
            else if (name == "traj_inertia") {
                auto values = param.as_double_array();
                if (values.size() != 2) {
                    RCLCPP_ERROR(this->get_logger(), "traj_inertia expects [id, inertia]");
                    result.successful = false;
                    continue;
                }

                int node_id = static_cast<int>(values[0]);
                float inertia = static_cast<float>(values[1]);

                odrive_->SetTrajectoryInertia(node_id, inertia);
                RCLCPP_INFO(this->get_logger(),
                            "Updated inertia: motor %d -> %.4f", node_id, inertia);
            }
        }

        return result;
    }
};


int main(int argc, char *argv[])
{
    rclcpp::init(argc, argv);
    rclcpp::spin(std::make_shared<TrajectoryTuningNode>());
    rclcpp::shutdown();
    return 0;
}
