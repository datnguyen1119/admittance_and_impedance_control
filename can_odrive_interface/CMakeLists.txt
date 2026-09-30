cmake_minimum_required(VERSION 3.8)
project(can_odrive_interface)

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# === Find ROS2 packages ===
find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)
find_package(std_msgs REQUIRED)

# === Build the shared library ===
add_library(${PROJECT_NAME} SHARED
  src/odrive_interface.cpp
)

target_include_directories(${PROJECT_NAME} PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include>
)

target_compile_features(${PROJECT_NAME} PUBLIC cxx_std_17)

ament_target_dependencies(${PROJECT_NAME}
  rclcpp
  std_msgs
)

# === Export library interface ===
ament_export_include_directories(include)
ament_export_libraries(${PROJECT_NAME})
ament_export_dependencies(rclcpp std_msgs)

# === Install library ===
install(
  TARGETS ${PROJECT_NAME}
  EXPORT export_${PROJECT_NAME}
  ARCHIVE DESTINATION lib
  LIBRARY DESTINATION lib
  RUNTIME DESTINATION bin
)

# === Build test/demo nodes ===
foreach(node IN ITEMS velocity position torque read_encoder)
  add_executable(${node}_node test/test_${node}.cpp)
  target_link_libraries(${node}_node ${PROJECT_NAME})
  target_compile_features(${node}_node PUBLIC cxx_std_17)
  ament_target_dependencies(${node}_node rclcpp std_msgs)
  install(TARGETS ${node}_node DESTINATION lib/${PROJECT_NAME})
endforeach()

add_executable(pid_tuning test/pid_tuning_node.cpp)
target_link_libraries(pid_tuning ${PROJECT_NAME})
target_compile_features(pid_tuning PUBLIC cxx_std_17)
ament_target_dependencies(pid_tuning rclcpp std_msgs)
install(TARGETS pid_tuning DESTINATION lib/${PROJECT_NAME})

# === Install headers ===
install(DIRECTORY include/ DESTINATION include)

ament_package()
