#include "rehab_robot_ver2_control/node_impedanceControl.hpp"

#include <cmath>
#include <memory>

using namespace std::chrono_literals;

namespace {

constexpr int MOTOR_ID = 2;
constexpr double REV_TO_RAD = M_PI / 4.0;
constexpr double RAD_TO_REV = 4.0 / M_PI;
constexpr double CONTROL_LOOP_DT_SEC = 0.001;
constexpr double GEARBOX_RATIO = 1.0;

inline double clamp(double x, double lo, double hi) {
  return (x < lo) ? lo : (x > hi) ? hi : x;
}
}  // namespace

ImpedanceControlNode::ImpedanceControlNode(): rclcpp::Node("impedance_control_node")
{
  odrive_functions_ = std::make_shared<can_odrive_interface::ODriveInterface>();

  this->declare_parameter("mass_inertia", 0.5);
  this->declare_parameter("damping_coefficient", 1.5);
  this->declare_parameter("stiffness_coefficient", 1.5);
  this->declare_parameter("robot_and_arm_mass", 0.58);
  this->declare_parameter("gravity_acceleration", 9.81);
  this->declare_parameter("center_of_mass_distance", 0.179);
  this->declare_parameter("velocity_limit", 10.0);

  initialize_motors();

  timer_ = this->create_wall_timer(std::chrono::milliseconds(1), std::bind(&ImpedanceControlNode::impedance_callback_loop, this));

  RCLCPP_INFO(this->get_logger(), "Impedance control node initialized (motor ID: %d).", MOTOR_ID);
}

ImpedanceControlNode::initialize_motors() {
  float initial_velocity = 0.0f;
  odrive_functions_->InitVelocityMode(MOTOR_ID);
  odrive_functions_->SendVelocityCommand(MOTOR_ID, initial_velocity);
}

double ImpedanceControlNode::compute_gravity_compensation(double current_position_rad) {
  const double robot_and_arm_mass = this->get_parameter("robot_and_arm_mass").as_double();
  const double gravity_acceleration = this->get_parameter("gravity_acceleration").as_double();
  const double center_of_mass_distance = this->get_parameter("center_of_mass_distance").as_double();
  const double gravity_compensation_torque = robot_and_arm_mass * gravity_acceleration * center_of_mass_distance * std::sin(current_position_rad);
  return gravity_compensation_torque;
}

double ImpedanceControlNode::compute_impedance_control(
    double q,
    double dq,
    double tau_gravity) {

  const double mass_inertia = this->get_parameter("mass_inertia").as_double();
  const double damping_coefficient = this->get_parameter("damping_coefficient").as_double();
  const double stiffness_coefficient = this->get_parameter("stiffness_coefficient").as_double();
  const double velocity_limit = this->get_parameter("velocity_limit").as_double();

  const double dt = CONTROL_LOOP_DT_SEC;
  const double q_ref = 0.0;
  static double q_prev = q;
  static double q
