from launch import LaunchDescription
from launch_ros.actions import Node
from launch.actions import ExecuteProcess
import os

def generate_launch_description():
    urdf_file = '/home/dat/Workspace/gravitational_compensation/src/rehab_robot_ver2/urdf/rehab_robot_ver2.urdf'
    with open(urdf_file, 'r') as infp:
        robot_desc = infp.read()

    rviz_config_file = '/home/dat/Workspace/gravitational_compensation/src/rviz_custom_plugins/rviz_config_file/rehab_robot.rviz'
    if not os.path.exists(rviz_config_file):
        rviz_config_file = ''  # dùng config mặc định nếu chưa có

    return LaunchDescription([
        #kinematics_solver
        Node(
            package='rehab_robot_ver2_control',
            executable='kinematics',
            name='kinematics',
            output='screen',
        ),
        # Robot State Publisher
        Node(
            package='robot_state_publisher',
            executable='robot_state_publisher',
            name='robot_state_publisher',
            output='screen',
            parameters=[{'robot_description': robot_desc}]
        ),

        # Static transform world -> dummy_link
        Node(
            package='tf2_ros',
            executable='static_transform_publisher',
            arguments=['0', '0', '0', '0', '0', '0', '1', 'world', 'base_footprint'],
        ),

        # Mở RViz2 với config nếu có
        ExecuteProcess(
            cmd=['rviz2', '-d', rviz_config_file] if rviz_config_file else ['rviz2'],
            output='screen'
        ),
    ])
