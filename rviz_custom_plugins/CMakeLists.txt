cmake_minimum_required(VERSION 3.8)
project(rviz_custom_plugins)

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# ==============================
# Dependencies
# ==============================
find_package(ament_cmake_ros REQUIRED)
find_package(pluginlib REQUIRED)
find_package(rviz_common REQUIRED)
find_package(rviz_rendering REQUIRED)
find_package(rviz_default_plugins REQUIRED)
find_package(Qt5 REQUIRED COMPONENTS Widgets)
find_package(std_msgs REQUIRED)
find_package(geometry_msgs REQUIRED)
find_package(rclcpp REQUIRED)
find_package(ament_index_cpp REQUIRED)
find_package(rosidl_default_generators REQUIRED)
find_package(moveit_core REQUIRED)
find_package(moveit_ros_planning REQUIRED)
find_package(srdfdom REQUIRED)
find_package(sensor_msgs REQUIRED)
find_package(moveit_msgs REQUIRED)
find_package(tf2 REQUIRED)
find_package(tf2_geometry_msgs REQUIRED)


# ==============================
# Message generation
# ==============================
rosidl_generate_interfaces(${PROJECT_NAME}
  "msg/KinematicsFKRequest.msg"
  "msg/KinematicsFKResponse.msg"
  "msg/KinematicsIKRequest.msg"
  "msg/KinematicsIKResponse.msg"
  DEPENDENCIES std_msgs geometry_msgs
)

ament_export_dependencies(rosidl_default_runtime)

# ==============================
# Qt setup
# ==============================
set(CMAKE_AUTOMOC ON)

qt5_wrap_cpp(MOC_FILES
  include/${PROJECT_NAME}/kinematics_panel.hpp
)


# ==============================
# Build plugin library
# ==============================
add_library(kinematics_panel SHARED
  src/kinematics_panel.cpp
  ${MOC_FILES}
)

target_include_directories(kinematics_panel PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include>
)

ament_target_dependencies(kinematics_panel
  pluginlib
  rclcpp
  rviz_common
  rviz_rendering
  rviz_default_plugins
  std_msgs
  geometry_msgs
  ament_index_cpp
  moveit_core
  moveit_ros_planning
  srdfdom
  sensor_msgs
  moveit_msgs
  tf2
  tf2_geometry_msgs
)

target_link_libraries(kinematics_panel Qt5::Widgets)

# Link message typesupport (để plugin có thể sử dụng msg)
rosidl_get_typesupport_target(cpp_typesupport_target ${PROJECT_NAME} "rosidl_typesupport_cpp")
target_link_libraries(kinematics_panel "${cpp_typesupport_target}")

# ==============================
# Installation
# ==============================
install(TARGETS kinematics_panel
  EXPORT export_${PROJECT_NAME}
  ARCHIVE DESTINATION lib
  LIBRARY DESTINATION lib
  RUNTIME DESTINATION bin
)

install(DIRECTORY include/
  DESTINATION include
)
install(DIRECTORY launch
        DESTINATION share/${PROJECT_NAME}/
)


install(FILES plugin_description.xml
  DESTINATION share/${PROJECT_NAME}
)

# ==============================
# Export for pluginlib
# ==============================
ament_export_include_directories(include)
ament_export_targets(export_${PROJECT_NAME})
pluginlib_export_plugin_description_file(rviz_common plugin_description.xml)


# ==============================
# Lint / Test
# ==============================
if(BUILD_TESTING)
  find_package(ament_lint_auto REQUIRED)
  set(ament_cmake_copyright_FOUND TRUE)
  set(ament_cmake_cpplint_FOUND TRUE)
  ament_lint_auto_find_test_dependencies()
endif()

ament_package()
