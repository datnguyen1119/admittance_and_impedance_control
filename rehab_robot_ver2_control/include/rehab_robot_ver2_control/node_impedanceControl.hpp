#ifndef IMPEDANCE_CONTROL_HPP_
#define IMPEDANCE_CONTROL_HPP_

#include <rclcpp/rclcpp.hpp>
#include <can_odrive_interface/odrive_interface.hpp>

class ImpedanceControlNode : public rclcpp::Node {
public:
    explicit ImpedanceControlNode();

    void initialize_motors();

    double compute_gravity_compensation(double current_position_rad);

    double compute_impedance_control(double q, double dq, double tau_gravity);

    void impedance_callback_loop();

private:
    std::shared_ptr<can_odrive_interface::ODriveInterface> odrive_functions_;
    rclcpp::TimerBase::SharedPtr timer_;
};
#endif // IMPEDANCE_CONTROL_HPP_