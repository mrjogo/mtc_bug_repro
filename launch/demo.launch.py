#!/usr/bin/env python3
"""
Minimal launch file for MoveIt Task Constructor bug reproduction.
Launches move_group with necessary configuration files.
"""

from launch import LaunchDescription
from launch.substitutions import PathJoinSubstitution, Command
from launch_ros.actions import Node
from launch_ros.substitutions import FindPackageShare


def generate_launch_description():
    """Generate launch description for minimal MoveIt setup."""

    # Package name
    package_name = 'mtc_bug_repro'

    # URDF path
    robot_description_content = Command([
        'cat ',
        PathJoinSubstitution([
            FindPackageShare(package_name),
            'urdf',
            'robot.urdf'
        ])
    ])
    robot_description = {'robot_description': robot_description_content}

    # SRDF path
    robot_description_semantic_content = Command([
        'cat ',
        PathJoinSubstitution([
            FindPackageShare(package_name),
            'config',
            'robot.srdf'
        ])
    ])
    robot_description_semantic = {'robot_description_semantic': robot_description_semantic_content}

    # Joint limits config path
    joint_limits_yaml = PathJoinSubstitution([
        FindPackageShare(package_name),
        'config',
        'joint_limits.yaml'
    ])

    # OMPL planning config path
    ompl_planning_yaml = PathJoinSubstitution([
        FindPackageShare(package_name),
        'config',
        'ompl_planning.yaml'
    ])

    # MoveIt controllers config path
    moveit_controllers_yaml = PathJoinSubstitution([
        FindPackageShare(package_name),
        'config',
        'moveit_controllers.yaml'
    ])

    moveit_cpp_yaml_params = (
        PathJoinSubstitution(
            [
                FindPackageShare(package_name),
                "config",
                "moveit_cpp.yaml",
            ]
        ),
    )

    # Move group node
    move_group_node = Node(
        package='moveit_ros_move_group',
        executable='move_group',
        output='screen',
        parameters=[
            robot_description,
            robot_description_semantic,
            moveit_cpp_yaml_params,
            joint_limits_yaml,
            ompl_planning_yaml,
            moveit_controllers_yaml,
            {'use_sim_time': True},
        ],
    )

    # Robot state publisher node
    robot_state_publisher_node = Node(
        package='robot_state_publisher',
        executable='robot_state_publisher',
        output='screen',
        parameters=[
            robot_description,
            {'use_sim_time': True}
        ],
    )

    return LaunchDescription([
        robot_state_publisher_node,
        move_group_node,
    ])
