// #ifndef RVIZ_CUSTOM_PLUGINS__KINEMATICS_PANEL_HPP_
// #define RVIZ_CUSTOM_PLUGINS__KINEMATICS_PANEL_HPP_

// #include <rclcpp/rclcpp.hpp>
// #include <rviz_common/panel.hpp>
// #include <rviz_common/display_context.hpp>
// #include <QPushButton>
// #include <QLineEdit>
// #include <QLabel>
// #include <QVBoxLayout>
// #include <QHBoxLayout>
// #include <QString>
// #include <vector>
// #include <rviz_common/ros_integration/ros_node_abstraction_iface.hpp>
// #include <sensor_msgs/msg/joint_state.hpp>   // ADD nếu chưa có


// // Include message type
// #include "rviz_custom_plugins/msg/kinematics_request.hpp"

// namespace rviz_custom_plugins
// {

// class KinematicsPanel : public rviz_common::Panel
// {
//   Q_OBJECT
// public:
//   explicit KinematicsPanel(QWidget *parent = nullptr);
//   ~KinematicsPanel() override;

//   void onInitialize() override;

// protected Q_SLOTS:
//   void sendRequest();

// protected:
//   QLineEdit *joint_names_edit_;
//   QLineEdit *joint_positions_edit_;
//   QLineEdit *frame_id_edit_;
//   QPushButton *send_button_;

//   std::shared_ptr<rviz_common::ros_integration::RosNodeAbstractionIface> node_abstraction_;
//   rclcpp::Publisher<rviz_custom_plugins::msg::KinematicsRequest>::SharedPtr publisher_;
//   rclcpp::Node::SharedPtr node_;
//   rclcpp::Publisher<sensor_msgs::msg::JointState>::SharedPtr joint_state_pub_; // ADD

// };

// }  // namespace rviz_custom_plugins

// #endif  // RVIZ_CUSTOM_PLUGINS__KINEMATICS_PANEL_HPP_

//-------------------------------new code-------------------------------//
#ifndef RVIZ_CUSTOM_PLUGINS__KINEMATICS_PANEL_HPP_
#define RVIZ_CUSTOM_PLUGINS__KINEMATICS_PANEL_HPP_

#include <rclcpp/rclcpp.hpp>
#include <rviz_common/panel.hpp>
#include <rviz_common/display_context.hpp>

#include <QPushButton>
#include <QLineEdit>
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QString>
#include <QSlider>
#include <QDoubleSpinBox>

#include <vector>
#include <rviz_common/ros_integration/ros_node_abstraction_iface.hpp>
#include <sensor_msgs/msg/joint_state.hpp>
#include <rclcpp/timer.hpp>

#include "rviz_custom_plugins/msg/kinematics_fk_request.hpp"
#include "rviz_custom_plugins/msg/kinematics_fk_response.hpp"
#include "rviz_custom_plugins/msg/kinematics_ik_request.hpp"     // NEW
#include "rviz_custom_plugins/msg/kinematics_ik_response.hpp"    // NEW
#include <geometry_msgs/msg/pose.hpp>


namespace rviz_custom_plugins
{

class KinematicsPanel : public rviz_common::Panel
{
  Q_OBJECT
public:
  explicit KinematicsPanel(QWidget *parent = nullptr);
  ~KinematicsPanel() override;

  void onInitialize() override;

protected Q_SLOTS:
  void sendFKRequest();       // FK
  void sendIKRequest();     // IK  // NEW
  void updateFKResult(const geometry_msgs::msg::Pose &pose);
  void updateIKResult(const std::vector<std::string> &names,
                      const std::vector<double> &values,
                      const std::string &status);

protected:

  // FK UI
  QSlider *base_slider_;
  QDoubleSpinBox *base_spin_;

  QSlider *shoulder_slider_;
  QDoubleSpinBox *shoulder_spin_;

  QSlider *elbow_slider_;
  QDoubleSpinBox *elbow_spin_;

  QPushButton *send_button_;
  QLabel* result_label_;

  // IK UI  (NEW)
  QDoubleSpinBox *x_input_;
  QDoubleSpinBox *y_input_;
  QDoubleSpinBox *z_input_;
  QDoubleSpinBox *roll_input_;   // NEW
  QDoubleSpinBox *pitch_input_;  // NEW
  QDoubleSpinBox *yaw_input_;    // NEW
  QPushButton *send_ik_button_;
  QLabel* ik_result_label_;

  // State
  std::vector<std::string> current_joint_names_;
  std::vector<double> current_joint_positions_;
  rclcpp::TimerBase::SharedPtr timer_;

  std::shared_ptr<rviz_common::ros_integration::RosNodeAbstractionIface> node_abstraction_;
  rclcpp::Node::SharedPtr node_;

  rclcpp::Publisher<sensor_msgs::msg::JointState>::SharedPtr joint_state_pub_;

  // ROS publishers and subscribers
  rclcpp::Publisher<rviz_custom_plugins::msg::KinematicsFKRequest>::SharedPtr fk_pub_;
  rclcpp::Publisher<rviz_custom_plugins::msg::KinematicsIKRequest>::SharedPtr ik_pub_;

  rclcpp::Subscription<rviz_custom_plugins::msg::KinematicsFKResponse>::SharedPtr fk_sub_;
  rclcpp::Subscription<rviz_custom_plugins::msg::KinematicsIKResponse>::SharedPtr ik_sub_;


  void publishJointState();
  void handleFKResult(const rviz_custom_plugins::msg::KinematicsFKResponse::SharedPtr msg);
  void handleIKResult(const rviz_custom_plugins::msg::KinematicsIKResponse::SharedPtr msg);

  geometry_msgs::msg::Pose last_fk_pose_;

  QPushButton *push_fk_to_ik_button_;
  void pushFKToIK();


};

}  // namespace rviz_custom_plugins

#endif  // RVIZ_CUSTOM_PLUGINS__KINEMATICS_PANEL_HPP_
