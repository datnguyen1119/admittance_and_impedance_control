#include "rehab_robot_ver2_control/node_gravityCompensation_elbowJoint.hpp"

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

GravityCompensationNode::GravityCompensationNode(): rclcpp::Node("gravity_compensation_elbow_node")
{
  odrive_functions_ = std::make_shared<can_odrive_interface::ODriveInterface>();

  this->declare_parameter("mass_inertia", 0.5);
  this->declare_parameter("damping_coefficient", 1.5);
  this->declare_parameter("stiffness_coefficient", 1.5);
  this->declare_parameter("velocity_feedforward_gain", 1.0);
  this->declare_parameter("robot_and_arm_mass", 2.12);
  // this->declare_parameter("robot_and_arm_mass", 0.58);
  this->declare_parameter("gravity_acceleration", 9.81);
  this->declare_parameter("center_of_mass_distance", 0.179);
  this->declare_parameter("velocity_limit", 10.0);

  initialize_odrive();

  timer_ = this->create_wall_timer(std::chrono::milliseconds(1), std::bind(&GravityCompensationNode::pid_admittance_callback_loop, this));

  RCLCPP_INFO(this->get_logger(), "Elbow gravity compensation node initialized (motor ID: %d).", MOTOR_ID);
}

void GravityCompensationNode::initialize_odrive() {
  odrive_functions_->InitVelocityMode(MOTOR_ID);

  // Set initial velocity to 0
  odrive_functions_->SendVelocityCommand(MOTOR_ID, 0.0f);
}

double GravityCompensationNode::compute_gravity_compensation(double current_position_rad) {
  const double robot_and_arm_mass = this->get_parameter("robot_and_arm_mass").as_double();
  const double gravity_acceleration = this->get_parameter("gravity_acceleration").as_double();
  const double center_of_mass_distance = this->get_parameter("center_of_mass_distance").as_double();
  const double gravity_compensation_torque = robot_and_arm_mass * gravity_acceleration * center_of_mass_distance * std::sin(current_position_rad);
  return gravity_compensation_torque;
}

double GravityCompensationNode::GravityCompensationBlock(double q) {
  return compute_gravity_compensation(q);
}

double GravityCompensationNode::PidAdmittanceBlock(double delta_tau) {
  static double q_cmd = 0.0;
  static double qd_cmd = 0.0;

  const double mass_inertia = this->get_parameter("mass_inertia").as_double();
  const double damping_coefficient = this->get_parameter("damping_coefficient").as_double();
  const double stiffness_coefficient = this->get_parameter("stiffness_coefficient").as_double();
  const double velocity_limit = this->get_parameter("velocity_limit").as_double();

  const double q_ref = 0.0;  // Assuming reference position is 0

  // tau_ext = delta_tau (assuming no filter_deadband for now)
  const double tau_ext = delta_tau;

  // e = q_cmd - q_ref
  const double e = q_cmd - q_ref;

  // qdd = inv(M) * (tau_ext - B*qd_cmd - K*e)
  const double qdd = (1.0 / mass_inertia) * (tau_ext - damping_coefficient * qd_cmd - stiffness_coefficient * e);

  // qd_cmd += qdd * dt
  qd_cmd += qdd * CONTROL_LOOP_DT_SEC;

  // clamp qd_cmd
  qd_cmd = clamp(qd_cmd, -velocity_limit, velocity_limit);

  // q_cmd += qd_cmd * dt (for next e calculation)
  q_cmd += qd_cmd * CONTROL_LOOP_DT_SEC;

  // return qd_cmd
  return qd_cmd;
}

void GravityCompensationNode::pid_admittance_callback_loop() {
  const double velocity_feedforward_gain = this->get_parameter("velocity_feedforward_gain").as_double();

  odrive_functions_->ReadEncoder(MOTOR_ID);
  odrive_functions_->ReadTorque(MOTOR_ID);

  float encoder_turn_revolutions = 0.0f, encoder_velocity_rev_per_sec = 0.0f;
  odrive_functions_->get_encoder_data_nolog(MOTOR_ID, encoder_turn_revolutions, encoder_velocity_rev_per_sec);

  const double q = static_cast<double>(encoder_turn_revolutions) * REV_TO_RAD / GEARBOX_RATIO;
  const double dq_dt = static_cast<double>(encoder_velocity_rev_per_sec) * REV_TO_RAD / GEARBOX_RATIO;

  float measured_torque_float = 0.0f;
  odrive_functions_->get_torque_data_nolog(MOTOR_ID, measured_torque_float);
  const double tau = static_cast<double>(measured_torque_float) * GEARBOX_RATIO;

  const double tau_gravity = GravityCompensationBlock(q);
  const double delta_tau_theoretical = tau - tau_gravity;
  const double delta_tau = -delta_tau_theoretical;  // external torque

  const double velocity_admittance = PidAdmittanceBlock(delta_tau);
  const double velocity_feedforward = velocity_feedforward_gain * dq_dt;
  const double total_velocity_rad_per_sec = velocity_admittance + velocity_feedforward;

  const float total_velocity_rev_per_sec = static_cast<float>(total_velocity_rad_per_sec * RAD_TO_REV);

  odrive_functions_->SendVelocityCommand(MOTOR_ID, total_velocity_rev_per_sec);
}

int main(int argc, char *argv[]) {
    rclcpp::init(argc, argv);
    rclcpp::spin(std::make_shared<GravityCompensationNode>());
    rclcpp::shutdown();
    return 0;
}

