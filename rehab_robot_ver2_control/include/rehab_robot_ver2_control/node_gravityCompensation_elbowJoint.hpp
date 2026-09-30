#ifndef GRAVITY_COMPENSATION_HPP_
#define GRAVITY_COMPENSATION_HPP_

#include <rclcpp/rclcpp.hpp>
#include <can_odrive_interface/odrive_interface.hpp>

class GravityCompensationNode : public rclcpp::Node {
public:
    explicit GravityCompensationNode();

    void initialize_odrive();

    double compute_gravity_compensation(double current_position_rad);

    double GravityCompensationBlock(double q);

    double PidAdmittanceBlock(double delta_tau);

    void pid_admittance_callback_loop();

private:
    std::shared_ptr<can_odrive_interface::ODriveInterface> odrive_functions_;
    rclcpp::TimerBase::SharedPtr timer_;
};
#endif // GRAVITY_COMPENSATION_HPP_