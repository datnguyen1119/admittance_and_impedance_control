// #include "rviz_custom_plugins/kinematics_panel.hpp"
// #include <pluginlib/class_list_macros.hpp>
// #include <sstream>
// #include <rviz_common/ros_integration/ros_node_abstraction_iface.hpp>

// #include <sensor_msgs/msg/joint_state.hpp>


// namespace rviz_custom_plugins
// {

// KinematicsPanel::KinematicsPanel(QWidget *parent)
// : rviz_common::Panel(parent)
// {
//   // Tạo giao diện
//   auto layout = new QVBoxLayout(this);

//   layout->addWidget(new QLabel("Joint Names (comma-separated):"));
//   joint_names_edit_ = new QLineEdit;
//   layout->addWidget(joint_names_edit_);

//   layout->addWidget(new QLabel("Joint Positions (comma-separated):"));
//   joint_positions_edit_ = new QLineEdit;
//   layout->addWidget(joint_positions_edit_);

//   layout->addWidget(new QLabel("Frame ID:"));
//   frame_id_edit_ = new QLineEdit("base_link");
//   layout->addWidget(frame_id_edit_);

//   send_button_ = new QPushButton("Send Request");
//   layout->addWidget(send_button_);

//   setLayout(layout);

//   connect(send_button_, &QPushButton::released, this, &KinematicsPanel::sendRequest);

// }

// KinematicsPanel::~KinematicsPanel() = default;

// void KinematicsPanel::onInitialize()
// {
//   node_abstraction_ = getDisplayContext()->getRosNodeAbstraction().lock();
//   node_ = node_abstraction_->get_raw_node();

//   publisher_ = node_->create_publisher<rviz_custom_plugins::msg::KinematicsFKRequest>(
//     "/kinematics/request", 10);

//   // ADD: Publisher cho joint_states
//   joint_state_pub_ = node_->create_publisher<sensor_msgs::msg::JointState>(
//     "/joint_states", 10);
// }


// void KinematicsPanel::sendRequest()
// {
//   auto msg = rviz_custom_plugins::msg::KinematicsFKRequest();

//   // joint_names
//   QStringList joint_names = joint_names_edit_->text().split(",", Qt::SkipEmptyParts);
//   for (const auto &name : joint_names)
//     msg.joint_names.push_back(name.trimmed().toStdString());

//   // joint_positions
//   QStringList joint_positions = joint_positions_edit_->text().split(",", Qt::SkipEmptyParts);
//   for (const auto &val : joint_positions)
//     msg.joint_positions.push_back(val.trimmed().toDouble());

//   msg.frame_id = frame_id_edit_->text().toStdString();

//   publisher_->publish(msg);
//   RCLCPP_INFO(node_->get_logger(), "Sent KinematicsFKRequest with %zu joints", msg.joint_names.size());

//   // === ADD: Publish JointState để điều khiển RobotModel ===
//   sensor_msgs::msg::JointState js;
//   js.header.stamp = node_->get_clock()->now();
//   js.name = msg.joint_names;
//   js.position = msg.joint_positions;

//   joint_state_pub_->publish(js);
//   RCLCPP_INFO(node_->get_logger(), "Published JointState");
// }

// }  // namespace rviz_custom_plugins

// PLUGINLIB_EXPORT_CLASS(rviz_custom_plugins::KinematicsPanel, rviz_common::Panel)

//------------------------------new code------------------------------//

// #include "rviz_custom_plugins/kinematics_panel.hpp"
// #include <pluginlib/class_list_macros.hpp>
// #include <sstream>
// #include <rviz_common/ros_integration/ros_node_abstraction_iface.hpp>

// #include <sensor_msgs/msg/joint_state.hpp>


// namespace rviz_custom_plugins
// {

// KinematicsPanel::KinematicsPanel(QWidget *parent)
// : rviz_common::Panel(parent)
// {
//   auto layout = new QVBoxLayout(this);

//   // ===== BASE JOINT =====
//   layout->addWidget(new QLabel("Base Joint:"));
//   base_slider_ = new QSlider(Qt::Horizontal);
//   base_slider_->setRange(-90, 0);  // độ
//   base_spin_ = new QDoubleSpinBox;
//   base_spin_->setRange(-90, 0);
//   base_spin_->setDecimals(2);
//   auto base_layout = new QHBoxLayout;
//   base_layout->addWidget(base_slider_);
//   base_layout->addWidget(base_spin_);
//   layout->addLayout(base_layout);

//   // ===== SHOULDER JOINT =====
//   layout->addWidget(new QLabel("Shoulder Joint:"));
//   shoulder_slider_ = new QSlider(Qt::Horizontal);
//   shoulder_slider_->setRange(-90, 15);
//   shoulder_spin_ = new QDoubleSpinBox;
//   shoulder_spin_->setRange(-90, 15);
//   shoulder_spin_->setDecimals(2);
//   auto shoulder_layout = new QHBoxLayout;
//   shoulder_layout->addWidget(shoulder_slider_);
//   shoulder_layout->addWidget(shoulder_spin_);
//   layout->addLayout(shoulder_layout);

//   // ===== ELBOW JOINT =====
//   layout->addWidget(new QLabel("Elbow Joint:"));
//   elbow_slider_ = new QSlider(Qt::Horizontal);
//   elbow_slider_->setRange(0,90);
//   elbow_spin_ = new QDoubleSpinBox;
//   elbow_spin_->setRange(0,90);
//   elbow_spin_->setDecimals(2);
//   auto elbow_layout = new QHBoxLayout;
//   elbow_layout->addWidget(elbow_slider_);
//   elbow_layout->addWidget(elbow_spin_);
//   layout->addLayout(elbow_layout);

//   // Nút gửi
//   send_button_ = new QPushButton("Send Joint Angles");
//   layout->addWidget(send_button_);

//   setLayout(layout);

//   // Kết quả hiển thị
//   layout->addWidget(new QLabel("Output Joint Positions (rad):"));
//   result_label_ = new QLabel("---");
//   layout->addWidget(result_label_);


//   // ===== CONNECT SLIDERS <-> SPINBOX =====
//   connect(base_slider_, &QSlider::valueChanged,
//         this, [=](int v){ base_spin_->setValue(static_cast<double>(v)); });
//   connect(shoulder_slider_, &QSlider::valueChanged,
//         this, [=](int v){ shoulder_spin_->setValue(static_cast<double>(v)); });
//   connect(elbow_slider_, &QSlider::valueChanged,
//         this, [=](int v){ elbow_spin_->setValue(static_cast<double>(v)); });
  
//   connect(base_spin_, qOverload<double>(&QDoubleSpinBox::valueChanged),
//         this, [=](double v){ base_slider_->setValue(static_cast<int>(v)); });
//   connect(shoulder_spin_, qOverload<double>(&QDoubleSpinBox::valueChanged),
//         this, [=](double v){ shoulder_slider_->setValue(static_cast<int>(v)); });
//   connect(elbow_spin_, qOverload<double>(&QDoubleSpinBox::valueChanged),
//         this, [=](double v){ elbow_slider_->setValue(static_cast<int>(v)); });

//   // Gửi khi nhấn nút
//   connect(send_button_, &QPushButton::released, this, &KinematicsPanel::sendRequest);
// }


// KinematicsPanel::~KinematicsPanel() = default;

// void KinematicsPanel::onInitialize()
// {
//   node_abstraction_ = getDisplayContext()->getRosNodeAbstraction().lock();
//   node_ = node_abstraction_->get_raw_node();

//   publisher_ = node_->create_publisher<rviz_custom_plugins::msg::KinematicsFKRequest>(
//     "/kinematics/request", 10);

//   // ADD: Publisher cho joint_states
//   joint_state_pub_ = node_->create_publisher<sensor_msgs::msg::JointState>(
//     "/joint_states", 10);

//   // Đăng ký subscriber để nhận kết quả FK
//   result_sub_ = node_->create_subscription<rviz_custom_plugins::msg::KinematicsFKResponse>(
//     "/kinematics/result", 10,
//     std::bind(&KinematicsPanel::handleResult, this, std::placeholders::_1));


//   // ✅ Publish /joint_states mỗi 50ms (20 Hz)
//   timer_ = node_->create_wall_timer(
//       std::chrono::milliseconds(200),
//       std::bind(&KinematicsPanel::publishJointState, this));
// }


// void KinematicsPanel::sendRequest()
// {
//   // Danh sách cố định
//   current_joint_names_ = {"base_joint", "shoulder_joint", "elbow_joint"};
//   current_joint_positions_.clear();
//   current_joint_positions_.push_back(base_spin_->value() * M_PI/180.0);
//   current_joint_positions_.push_back(shoulder_spin_->value() * M_PI/180.0);
//   current_joint_positions_.push_back(elbow_spin_->value() * M_PI/180.0);


//   // Gửi message IK/FK request (nếu cần)
//   auto msg = rviz_custom_plugins::msg::KinematicsFKRequest();
//   msg.joint_names = current_joint_names_;
//   msg.joint_positions = current_joint_positions_;
//   msg.frame_id = "base_link";
//   publisher_->publish(msg);

//   RCLCPP_INFO(node_->get_logger(), "Updated joint targets.");
// }


// void KinematicsPanel::publishJointState()
// {
//   if (current_joint_names_.empty()) return;  // không có gì để publish

//   sensor_msgs::msg::JointState js;
//   js.header.stamp = node_->get_clock()->now();
//   js.name = current_joint_names_;
//   js.position = current_joint_positions_;

//   joint_state_pub_->publish(js);
// }

// void KinematicsPanel::handleResult(const rviz_custom_plugins::msg::KinematicsFKResponse::SharedPtr msg)
// {
//   const auto &p = msg->pose.position;
//   const auto &q = msg->pose.orientation;

//   QString text = QString("Position: [%.3f, %.3f, %.3f]\n"
//                          "Orientation (quat): [%.3f, %.3f, %.3f, %.3f]")
//       .arg(p.x).arg(p.y).arg(p.z)
//       .arg(q.x).arg(q.y).arg(q.z).arg(q.w);

//   result_label_->setText(text);
// }



// }  // namespace rviz_custom_plugins

// PLUGINLIB_EXPORT_CLASS(rviz_custom_plugins::KinematicsPanel, rviz_common::Panel)

// #include "rviz_custom_plugins/kinematics_panel.hpp"
// #include <pluginlib/class_list_macros.hpp>
// #include <sstream>
// #include <rviz_common/ros_integration/ros_node_abstraction_iface.hpp>
// #include <sensor_msgs/msg/joint_state.hpp>
// #include <QMetaObject>
// #include <QMetaType>
// #include <cmath>
// #include <tf2/LinearMath/Quaternion.h>
// #include <tf2/LinearMath/Matrix3x3.h>


// namespace rviz_custom_plugins
// {

// KinematicsPanel::KinematicsPanel(QWidget *parent)
// : rviz_common::Panel(parent)
// {
//   // Tạo layout nhưng setLayout ở cuối
//   auto layout = new QVBoxLayout();

//   // ===== BASE JOINT =====
//   layout->addWidget(new QLabel("Base Joint (deg):", this));
//   base_slider_ = new QSlider(Qt::Horizontal, this);
//   base_slider_->setRange(-90, 0);  // độ
//   base_spin_ = new QDoubleSpinBox(this);
//   base_spin_->setRange(-90, 0);
//   base_spin_->setDecimals(2);
//   auto base_layout = new QHBoxLayout();
//   base_layout->addWidget(base_slider_);
//   base_layout->addWidget(base_spin_);
//   layout->addLayout(base_layout);

//   // ===== SHOULDER JOINT =====
//   layout->addWidget(new QLabel("Shoulder Joint (deg):", this));
//   shoulder_slider_ = new QSlider(Qt::Horizontal, this);
//   shoulder_slider_->setRange(-90, 15);
//   shoulder_spin_ = new QDoubleSpinBox(this);
//   shoulder_spin_->setRange(-90, 15);
//   shoulder_spin_->setDecimals(2);
//   auto shoulder_layout = new QHBoxLayout();
//   shoulder_layout->addWidget(shoulder_slider_);
//   shoulder_layout->addWidget(shoulder_spin_);
//   layout->addLayout(shoulder_layout);

//   // ===== ELBOW JOINT =====
//   layout->addWidget(new QLabel("Elbow Joint (deg):", this));
//   elbow_slider_ = new QSlider(Qt::Horizontal, this);
//   elbow_slider_->setRange(0, 90);
//   elbow_spin_ = new QDoubleSpinBox(this);
//   elbow_spin_->setRange(0, 90);
//   elbow_spin_->setDecimals(2);
//   auto elbow_layout = new QHBoxLayout();
//   elbow_layout->addWidget(elbow_slider_);
//   elbow_layout->addWidget(elbow_spin_);
//   layout->addLayout(elbow_layout);

//   // Nút gửi
//   send_button_ = new QPushButton("Send Joint Angles", this);
//   layout->addWidget(send_button_);

//   // Kết quả hiển thị
//   layout->addWidget(new QLabel("Output (Pose or Joints):", this));
//   result_label_ = new QLabel("---", this);
//   result_label_->setWordWrap(true);
//   layout->addWidget(result_label_);

//   // ===== CONNECT SLIDERS <-> SPINBOX =====
//   // slider -> spinbox (int -> double)
//   connect(base_slider_, &QSlider::valueChanged,
//         this, [=](int v){ base_spin_->setValue(static_cast<double>(v)); });
//   connect(shoulder_slider_, &QSlider::valueChanged,
//         this, [=](int v){ shoulder_spin_->setValue(static_cast<double>(v)); });
//   connect(elbow_slider_, &QSlider::valueChanged,
//         this, [=](int v){ elbow_spin_->setValue(static_cast<double>(v)); });

//   // spinbox -> slider (double -> int)
//   connect(base_spin_, qOverload<double>(&QDoubleSpinBox::valueChanged),
//         this, [=](double v){ base_slider_->setValue(static_cast<int>(std::round(v))); });
//   connect(shoulder_spin_, qOverload<double>(&QDoubleSpinBox::valueChanged),
//         this, [=](double v){ shoulder_slider_->setValue(static_cast<int>(std::round(v))); });
//   connect(elbow_spin_, qOverload<double>(&QDoubleSpinBox::valueChanged),
//         this, [=](double v){ elbow_slider_->setValue(static_cast<int>(std::round(v))); });

//   // Gửi khi nhấn nút
//   connect(send_button_, &QPushButton::released, this, &KinematicsPanel::sendRequest);

//   // Cuối cùng set layout
//   setLayout(layout);
// }


// KinematicsPanel::~KinematicsPanel() = default;

// void KinematicsPanel::onInitialize()
// {
//   node_abstraction_ = getDisplayContext()->getRosNodeAbstraction().lock();
//   node_ = node_abstraction_->get_raw_node();

//   publisher_ = node_->create_publisher<rviz_custom_plugins::msg::KinematicsFKRequest>(
//     "/ForwardKinematics/request", 10);

//   // Publisher cho joint_states
//   joint_state_pub_ = node_->create_publisher<sensor_msgs::msg::JointState>(
//     "/joint_states", 10);

//   // Timer publish joint states (periodic) — bảo vệ dữ liệu trống
//   timer_ = node_->create_wall_timer(
//       std::chrono::milliseconds(50),
//       std::bind(&KinematicsPanel::publishJointState, this));

//   // Subscriber để nhận kết quả FK (pose)
//   // Sử dụng lambda để chuyển cập nhật UI vào luồng Qt (QueuedConnection)
//   result_sub_ = node_->create_subscription<rviz_custom_plugins::msg::KinematicsFKResponse>(
//     "/ForwardKinematics/result", 10,
//     [this](const rviz_custom_plugins::msg::KinematicsFKResponse::SharedPtr msg)
//     {
//       const auto &p = msg->pose.position;
//       const auto &q = msg->pose.orientation;

//       // Chuyển Quaternion -> RPY
//       tf2::Quaternion quat(q.x, q.y, q.z, q.w);
//       double roll, pitch, yaw;
//       tf2::Matrix3x3(quat).getRPY(roll, pitch, yaw);

//       QString text = QString("Position: [ %1, %2, %3 ]\nRPY: [ R=%4, P=%5, Y=%6 ]")
//                     .arg(p.x, 0, 'f', 4)
//                     .arg(p.y, 0, 'f', 4)
//                     .arg(p.z, 0, 'f', 4)
//                     .arg(roll, 0, 'f', 3)
//                     .arg(pitch, 0, 'f', 3)
//                     .arg(yaw, 0, 'f', 3);


//       // Update QLabel trong GUI thread
//       QMetaObject::invokeMethod(result_label_, "setText",
//                                 Qt::QueuedConnection,
//                                 Q_ARG(QString, text));
//     });

//   RCLCPP_INFO(node_->get_logger(), "KinematicsPanel initialized: publishers + subscriber ready");
// }


// void KinematicsPanel::sendRequest()
// {
//   // Danh sách cố định
//   current_joint_names_ = {"base_joint", "shoulder_joint", "elbow_joint"};
//   current_joint_positions_.clear();
//   current_joint_positions_.push_back(base_spin_->value() * M_PI/180.0);
//   current_joint_positions_.push_back(shoulder_spin_->value() * M_PI/180.0);
//   current_joint_positions_.push_back(elbow_spin_->value() * M_PI/180.0);

//   // Gửi message IK/FK request (nếu cần) — hiện panel đang gửi joint vector,
//   // nhưng nếu bạn chuyển sang gửi pose, chỉnh msg ở đây tương ứng.
//   auto msg = rviz_custom_plugins::msg::KinematicsFKRequest();
//   msg.joint_names = current_joint_names_;
//   msg.joint_positions = current_joint_positions_;
//   msg.frame_id = "base_link";
//   publisher_->publish(msg);

//   RCLCPP_INFO(node_->get_logger(), "Updated joint targets (and published request).");
// }


// void KinematicsPanel::publishJointState()
// {
//   if (current_joint_names_.empty() || current_joint_positions_.empty()) return;

//   sensor_msgs::msg::JointState js;
//   js.header.stamp = node_->get_clock()->now();
//   js.name = current_joint_names_;
//   js.position = current_joint_positions_;

//   joint_state_pub_->publish(js);
// }

// }  // namespace rviz_custom_plugins

// PLUGINLIB_EXPORT_CLASS(rviz_custom_plugins::KinematicsPanel, rviz_common::Panel)


#include "rviz_custom_plugins/kinematics_panel.hpp"
#include <pluginlib/class_list_macros.hpp>
#include <rviz_common/ros_integration/ros_node_abstraction_iface.hpp>
#include <sensor_msgs/msg/joint_state.hpp>
#include <QMetaObject>
#include <cmath>
#include <tf2/LinearMath/Quaternion.h>
#include <tf2/LinearMath/Matrix3x3.h>
#include <QGroupBox>
#include <QDoubleSpinBox>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QSlider>
#include <QGridLayout>

namespace rviz_custom_plugins
{

KinematicsPanel::KinematicsPanel(QWidget *parent)
: rviz_common::Panel(parent)
{
  auto layout = new QVBoxLayout();

  // ========================= FK UI =============================
  auto fk_group = new QGroupBox("Forward Kinematics (Input Joint Angles)");
  auto fk_layout = new QVBoxLayout();

  // BASE
  fk_layout->addWidget(new QLabel("Base Joint (deg):", this));
  base_slider_ = new QSlider(Qt::Horizontal, this);
  base_slider_->setRange(-90, 0);
  base_spin_ = new QDoubleSpinBox(this);
  base_spin_->setRange(-90, 0);
  base_spin_->setDecimals(2);
  auto base_row = new QHBoxLayout();
  base_row->addWidget(base_slider_);
  base_row->addWidget(base_spin_);
  fk_layout->addLayout(base_row);

  // SHOULDER
  fk_layout->addWidget(new QLabel("Shoulder Joint (deg):", this));
  shoulder_slider_ = new QSlider(Qt::Horizontal, this);
  shoulder_slider_->setRange(-22, 68);
  shoulder_spin_ = new QDoubleSpinBox(this);
  shoulder_spin_->setRange(-22, 68);
  shoulder_spin_->setDecimals(2);
  auto shoulder_row = new QHBoxLayout();
  shoulder_row->addWidget(shoulder_slider_);
  shoulder_row->addWidget(shoulder_spin_);
  fk_layout->addLayout(shoulder_row);

  // ELBOW
  fk_layout->addWidget(new QLabel("Elbow Joint (deg):", this));
  elbow_slider_ = new QSlider(Qt::Horizontal, this);
  elbow_slider_->setRange(-90, 0);
  elbow_spin_ = new QDoubleSpinBox(this);
  elbow_spin_->setRange(-90, 0);
  elbow_spin_->setDecimals(2);
  auto elbow_row = new QHBoxLayout();
  elbow_row->addWidget(elbow_slider_);
  elbow_row->addWidget(elbow_spin_);
  fk_layout->addLayout(elbow_row);

  send_button_ = new QPushButton("Compute Forward Kinematics");
  fk_layout->addWidget(send_button_);

  push_fk_to_ik_button_ = new QPushButton("Use FK Result for IK Input");
  fk_layout->addWidget(push_fk_to_ik_button_);

  connect(push_fk_to_ik_button_, &QPushButton::released, this, &KinematicsPanel::pushFKToIK);


  fk_group->setLayout(fk_layout);
  layout->addWidget(fk_group);

  // ========================= IK UI =============================
  auto ik_group = new QGroupBox("Inverse Kinematics (Input Desired Pose)");
  auto ik_layout = new QGridLayout();

  ik_layout->addWidget(new QLabel("X:"), 0, 0);
  x_input_ = new QDoubleSpinBox(); x_input_->setRange(-1.0, 1.0); x_input_->setDecimals(4);
  ik_layout->addWidget(x_input_, 0, 1);

  ik_layout->addWidget(new QLabel("Y:"), 1, 0);
  y_input_ = new QDoubleSpinBox(); y_input_->setRange(-1.0, 1.0); y_input_->setDecimals(4);
  ik_layout->addWidget(y_input_, 1, 1);

  ik_layout->addWidget(new QLabel("Z:"), 2, 0);
  z_input_ = new QDoubleSpinBox(); z_input_->setRange(-1.5, 1.5); z_input_->setDecimals(4);
  ik_layout->addWidget(z_input_, 2, 1);

  ik_layout->addWidget(new QLabel("Roll:"), 0, 2);
  roll_input_ = new QDoubleSpinBox(); roll_input_->setRange(-M_PI, M_PI); roll_input_->setDecimals(4);
  ik_layout->addWidget(roll_input_, 0, 3);

  ik_layout->addWidget(new QLabel("Pitch:"), 1, 2);
  pitch_input_ = new QDoubleSpinBox(); pitch_input_->setRange(-M_PI, M_PI); pitch_input_->setDecimals(4);
  ik_layout->addWidget(pitch_input_, 1, 3);

  ik_layout->addWidget(new QLabel("Yaw:"), 2, 2);
  yaw_input_ = new QDoubleSpinBox(); yaw_input_->setRange(-M_PI, M_PI); yaw_input_->setDecimals(4);
  ik_layout->addWidget(yaw_input_, 2, 3);

  send_ik_button_ = new QPushButton("Solve Inverse Kinematics");
  ik_layout->addWidget(send_ik_button_, 3, 0, 1, 4);

  ik_group->setLayout(ik_layout);
  layout->addWidget(ik_group);
  
  // x_input_->setDecimals(4);
  // y_input_->setDecimals(4);
  // z_input_->setDecimals(4);
  // roll_input_->setDecimals(4);
  // pitch_input_->setDecimals(4);
  // yaw_input_->setDecimals(4);


  // ========================= OUTPUT BOX =========================
  auto out_group = new QGroupBox("Output");
  auto out_layout = new QVBoxLayout();
  result_label_ = new QLabel("---");
  result_label_->setWordWrap(true);
  out_layout->addWidget(result_label_);
  out_group->setLayout(out_layout);
  layout->addWidget(out_group);

  // Sync slider/spin
  connect(base_slider_, &QSlider::valueChanged, this, [=](int v){ base_spin_->setValue(v); });
  connect(shoulder_slider_, &QSlider::valueChanged, this, [=](int v){ shoulder_spin_->setValue(v); });
  connect(elbow_slider_, &QSlider::valueChanged, this, [=](int v){ elbow_spin_->setValue(v); });

  connect(base_spin_, qOverload<double>(&QDoubleSpinBox::valueChanged), this, [=](double v){ base_slider_->setValue(v); });
  connect(shoulder_spin_, qOverload<double>(&QDoubleSpinBox::valueChanged), this, [=](double v){ shoulder_slider_->setValue(v); });
  connect(elbow_spin_, qOverload<double>(&QDoubleSpinBox::valueChanged), this, [=](double v){ elbow_slider_->setValue(v); });

  // Bind send FK
  connect(send_button_, &QPushButton::released, this, &KinematicsPanel::sendFKRequest);

  // Bind send IK
  connect(send_ik_button_, &QPushButton::released, this, &KinematicsPanel::sendIKRequest);

  setLayout(layout);
}

KinematicsPanel::~KinematicsPanel()
{
  // Nếu không có gì đặc biệt thì để trống cũng được
}

void KinematicsPanel::onInitialize()
{
  node_abstraction_ = getDisplayContext()->getRosNodeAbstraction().lock();
  node_ = node_abstraction_->get_raw_node();

  fk_pub_ = node_->create_publisher<rviz_custom_plugins::msg::KinematicsFKRequest>("/ForwardKinematics/request",10);
  ik_pub_ = node_->create_publisher<rviz_custom_plugins::msg::KinematicsIKRequest>("/InverseKinematics/request",10);

  joint_state_pub_ = node_->create_publisher<sensor_msgs::msg::JointState>("/joint_states",10);

  timer_ = node_->create_wall_timer(std::chrono::milliseconds(50), std::bind(&KinematicsPanel::publishJointState,this));

  fk_sub_ = node_->create_subscription<rviz_custom_plugins::msg::KinematicsFKResponse>(
    "/ForwardKinematics/response",10,
    std::bind(&KinematicsPanel::handleFKResult, this, std::placeholders::_1)
  );


  ik_sub_ = node_->create_subscription<rviz_custom_plugins::msg::KinematicsIKResponse>(
    "/InverseKinematics/response",10,
    std::bind(&KinematicsPanel::handleIKResult, this, std::placeholders::_1)
  );
}

void KinematicsPanel::sendFKRequest()
{
  current_joint_names_ = {"base_joint","shoulder_joint","elbow_joint"};
  current_joint_positions_ = {
    base_spin_->value()*M_PI/180.0,
    shoulder_spin_->value()*M_PI/180.0,
    elbow_spin_->value()*M_PI/180.0
  };

  rviz_custom_plugins::msg::KinematicsFKRequest msg;
  msg.frame_id = "base_link";
  msg.joint_names = current_joint_names_;
  msg.joint_positions = current_joint_positions_;
  fk_pub_->publish(msg);
}

void KinematicsPanel::sendIKRequest()
{
  rviz_custom_plugins::msg::KinematicsIKRequest msg;
  msg.frame_id = "base_link";

  tf2::Quaternion q;
  q.setRPY(roll_input_->value(), pitch_input_->value(), yaw_input_->value());

  msg.target_pose.position.x = x_input_->value();
  msg.target_pose.position.y = y_input_->value();
  msg.target_pose.position.z = z_input_->value();
  msg.target_pose.orientation.x = q.x();
  msg.target_pose.orientation.y = q.y();
  msg.target_pose.orientation.z = q.z();
  msg.target_pose.orientation.w = q.w();

  ik_pub_->publish(msg);
}

void KinematicsPanel::handleFKResult(const rviz_custom_plugins::msg::KinematicsFKResponse::SharedPtr msg)
{
  const auto &pose = msg->pose;


  double roll, pitch, yaw;
  tf2::Matrix3x3(tf2::Quaternion(pose.orientation.x, pose.orientation.y,
                                pose.orientation.z, pose.orientation.w))
    .getRPY(roll,pitch,yaw);

  RCLCPP_INFO(node_->get_logger(), "Rviz receive FK Result Pose: [%.3f, %.3f, %.3f], RPY: [%.3f, %.3f, %.3f]",
              pose.position.x, pose.position.y, pose.position.z,
              roll, pitch, yaw);

  QString text = QString("FK Pose:\n [%1, %2, %3]\nRPY: [%4, %5, %6]")
      .arg(pose.position.x, 0, 'f', 3)
      .arg(pose.position.y, 0, 'f', 3)
      .arg(pose.position.z, 0, 'f', 3)
      .arg(roll, 0, 'f', 3)
      .arg(pitch, 0, 'f', 3)
      .arg(yaw, 0, 'f', 3);


  QMetaObject::invokeMethod(result_label_,"setText",Qt::QueuedConnection,Q_ARG(QString,text));
  last_fk_pose_ = msg->pose;
  RCLCPP_INFO(node_->get_logger(),
  "Saved FK pose: [%.3f, %.3f, %.3f]",
  last_fk_pose_.position.x,
  last_fk_pose_.position.y,
  last_fk_pose_.position.z);

}

void KinematicsPanel::handleIKResult(const rviz_custom_plugins::msg::KinematicsIKResponse::SharedPtr msg)
{
  // // ✅ Update current joint state so timer publishes IK result continuously
  // current_joint_names_ = msg->joint_names;
  // current_joint_positions_.clear();
  // for (auto v : msg->joint_positions)
  //   current_joint_positions_.push_back(v);

  // ===== Display text =====
  QString text = "IK Result (" + QString::fromStdString(msg->status) + "):\n";

  for(size_t i=0;i<msg->joint_names.size();i++){
    double rad = msg->joint_positions[i];
    double deg = rad * 180.0 / M_PI;

    text += QString("%1 = %2 rad (%3 deg)\n")
      .arg(QString::fromStdString(msg->joint_names[i]))
      .arg(rad, 0, 'f', 3)
      .arg(deg, 0, 'f', 1);
  }

  QMetaObject::invokeMethod(result_label_,"setText",Qt::QueuedConnection,Q_ARG(QString,text));
}

void KinematicsPanel::publishJointState()
{
  if(current_joint_names_.empty()) return;

  sensor_msgs::msg::JointState js;
  js.header.stamp = node_->get_clock()->now();
  js.name = current_joint_names_;
  js.position = current_joint_positions_;
  joint_state_pub_->publish(js);
}

void KinematicsPanel::pushFKToIK()
{
  // Lấy pos
  x_input_->setValue(last_fk_pose_.position.x);
  y_input_->setValue(last_fk_pose_.position.y);
  z_input_->setValue(last_fk_pose_.position.z);
  RCLCPP_INFO(node_->get_logger(), "z_input_ after setValue(): %.4f", z_input_->value());


  // Lấy orientation → convert sang RPY
  double roll, pitch, yaw;
  tf2::Matrix3x3(tf2::Quaternion(
      last_fk_pose_.orientation.x,
      last_fk_pose_.orientation.y,
      last_fk_pose_.orientation.z,
      last_fk_pose_.orientation.w
    )).getRPY(roll,pitch,yaw);

  roll_input_->setValue(roll);
  pitch_input_->setValue(pitch);
  yaw_input_->setValue(yaw);

  result_label_->setText("✅ FK Pose copied → IK Input");
  RCLCPP_INFO(node_->get_logger(), "IK Panel receive FK Result Pose: [%.3f, %.3f, %.3f], RPY: [%.3f, %.3f, %.3f]",
              x_input_->value(), y_input_->value(), z_input_->value(),
              roll_input_->value(), pitch_input_->value(), yaw_input_->value());
}

} // namespace rviz_custom_plugins

PLUGINLIB_EXPORT_CLASS(rviz_custom_plugins::KinematicsPanel, rviz_common::Panel)
