cmake_minimum_required(VERSION 3.8)
project(rehab_robot_ver2_control)

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# find dependencies
find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)
find_package(rclcpp_lifecycle REQUIRED)
find_package(orocos_kdl REQUIRED)
find_package(urdf REQUIRED)
find_package(kdl_parser REQUIRED)
find_package(rviz_custom_plugins REQUIRED)
find_package(Qt5 REQUIRED COMPONENTS Widgets)
find_package(rviz_common REQUIRED)
find_package(rviz_rendering REQUIRED)
find_package(rviz_default_plugins REQUIRED)
find_package(std_msgs REQUIRED)
find_package(geometry_msgs REQUIRED)
find_package(ament_index_cpp REQUIRED)
find_package(moveit_core REQUIRED)
find_package(moveit_ros_planning REQUIRED)
find_package(moveit_ros_planning_interface REQUIRED)
find_package(srdfdom REQUIRED)
find_package(urdfdom REQUIRED)
find_package(sensor_msgs REQUIRED)
find_package(tf2 REQUIRED)
find_package(tf2_geometry_msgs REQUIRED)
find_package(can_odrive_interface REQUIRED)
find_package(pinocchio REQUIRED)
find_package(Eigen3 REQUIRED)




add_executable(kinematics src/kinematics.cpp)
add_executable(motor_state_publisher src/motor_state_publisher.cpp)
add_executable(robot_controller src/robotController.cpp)
# add_executable(motor_state_publisher_lifecycle src/motor_state_publisher_lifecycle.cpp)
# add_executable(robot_controller_lifecycle src/robot_controller_lifecycle.cpp)
add_executable(velocity_config_node src/velocity_config_node.cpp)
add_executable(data_pub_node src/data_pub_node.cpp)
add_executable(torque_pub_node src/torque_pub_node.cpp)
add_executable(velocity_modes src/velocity_modes.cpp)
add_executable(gravCompTest test/gravCompTest.cpp)
add_executable(admittance_control src/admittance_control.cpp)
add_executable(admittance_control_2 src/admittance_control_2.cpp)
# add_executable(simple_motor_control test/simple_motor_control.cpp)
add_executable(simple_velocity_node test/velocityControlTest.cpp)
add_executable(node_gravity_compensation_elbow src/node_gravityCompensation_elbowJoint.cpp)

include_directories(
  include
  ${EIGEN3_INCLUDE_DIRS}
)


add_library(gravity_comp_lib
    src/gravityCompNode.cpp
)


target_include_directories(gravity_comp_lib PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)


ament_target_dependencies(kinematics
  rclcpp
  orocos_kdl
  urdf
  kdl_parser
  rviz_custom_plugins
  std_msgs
  geometry_msgs
  ament_index_cpp
  rviz_common
  rviz_rendering
  rviz_default_plugins
  moveit_core
  moveit_ros_planning
  moveit_ros_planning_interface
  srdfdom
  sensor_msgs
  tf2
  tf2_geometry_msgs
  can_odrive_interface
)
ament_target_dependencies(motor_state_publisher
  rclcpp
  std_msgs
  can_odrive_interface
)
ament_target_dependencies(robot_controller
  rclcpp
  can_odrive_interface
  rviz_custom_plugins
)
# ament_target_dependencies(motor_state_publisher_lifecycle
#   rclcpp
#   rclcpp_lifecycle
#   std_msgs
#   can_odrive_interface
# )
# ament_target_dependencies(robot_controller_lifecycle
#   rclcpp
#   rclcpp_lifecycle
#   can_odrive_interface
#   rviz_custom_plugins
# )

ament_target_dependencies(velocity_config_node
  rclcpp
  can_odrive_interface
)

ament_target_dependencies(data_pub_node
  rclcpp
  std_msgs
  # can_odrive_interface
)

ament_target_dependencies(torque_pub_node
  rclcpp
  std_msgs
  can_odrive_interface
)

ament_target_dependencies(velocity_modes
  rclcpp
  can_odrive_interface
  std_msgs
)

ament_target_dependencies(gravCompTest
  rclcpp
  std_msgs
  pinocchio
  can_odrive_interface
)

target_link_libraries(gravCompTest
  gravity_comp_lib
)

# ament_target_dependencies(simple_motor_control
#   rclcpp
#   can_odrive_interface
# )


ament_target_dependencies(gravity_comp_lib
  rclcpp
  can_odrive_interface
  pinocchio
)

ament_target_dependencies(admittance_control
  rclcpp
  can_odrive_interface
)

ament_target_dependencies(admittance_control_2
  rclcpp
  can_odrive_interface
)

ament_target_dependencies(simple_velocity_node
  rclcpp
  can_odrive_interface
)

ament_target_dependencies(node_gravity_compensation_elbow
  rclcpp
  can_odrive_interface
  # pinocchio
)

target_link_libraries(kinematics
  ${orocos_kdl_LIBRARIES}
  ${urdf_LIBRARIES}
  ${kdl_parser_LIBRARIES}
)

target_link_libraries(gravity_comp_lib
    pinocchio::pinocchio
)

install(TARGETS
  kinematics
  motor_state_publisher
  robot_controller
  # motor_state_publisher_lifecycle
  # robot_controller_lifecycle
  velocity_config_node
  velocity_modes
  data_pub_node
  torque_pub_node
  gravCompTest
  admittance_control
  admittance_control_2
  simple_velocity_node
  node_gravity_compensation_elbow
  
  DESTINATION lib/${PROJECT_NAME}
)

install(DIRECTORY include/
  DESTINATION include/
)

install(
  DIRECTORY launch
  DESTINATION share/${PROJECT_NAME}/
)


ament_package()
