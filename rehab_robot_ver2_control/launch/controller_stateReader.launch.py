from launch import LaunchDescription
from launch_ros.actions import Node
from launch.actions import ExecuteProcess
import os

def generate_launch_description():

    return LaunchDescription([
        #kinematics_solver
        Node(
            package='rehab_robot_ver2_control',
            executable='motor_state_publisher',
            name='motor_state_publisher',
            output='screen',
        ),
        # Robot State Publisher
         Node(
            package='rehab_robot_ver2_control',
            executable='robot_controller',
            name='robot_controller',
            output='screen',
        ),

    ])
