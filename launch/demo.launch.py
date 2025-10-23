#!/usr/bin/env python3
"""
Minimal launch file for MoveIt Task Constructor bug reproduction.
Launches move_group with necessary configuration files.
"""

import os
from ament_index_python.packages import get_package_share_directory
from launch import LaunchDescription
from launch.substitutions import LaunchConfiguration
from launch_ros.actions import Node
from launch.conditions import IfCondition, UnlessCondition


def load_file(package_name, file_path):
    """Load a file from a package."""
    package_path = get_package_share_directory(package_name)
    absolute_file_path = os.path.join(package_path, file_path)
    try:
        with open(absolute_file_path, 'r') as file:
            return file.read()
    except EnvironmentError:
        return None


def load_yaml(package_name, file_path):
    """Load a YAML file from a package."""
    package_path = get_package_share_directory(package_name)
    absolute_file_path = os.path.join(package_path, file_path)
    try:
        import yaml
        with open(absolute_file_path, 'r') as file:
            return yaml.safe_load(file)
    except EnvironmentError:
        return None


def generate_launch_description():
    """Generate launch description for minimal MoveIt setup."""

    # Package name
    package_name = 'mtc_bug_repro'

    # Load URDF
    robot_description_content = load_file(package_name, 'urdf/robot.urdf')
    robot_description = {'robot_description': robot_description_content}

    # Load SRDF
    robot_description_semantic_content = load_file(package_name, 'config/robot.srdf')
    robot_description_semantic = {'robot_description_semantic': robot_description_semantic_content}

    # Kinematics config
    kinematics_yaml = load_yaml(package_name, 'config/kinematics.yaml')
    robot_description_kinematics = {'robot_description_kinematics': kinematics_yaml}

    # Joint limits config
    joint_limits_yaml = load_yaml(package_name, 'config/joint_limits.yaml')
    robot_description_planning = {'robot_description_planning': joint_limits_yaml}

    # Planning pipeline config
    planning_pipelines_config = {
        'planning_pipelines': ['ompl'],
        'default_planning_pipeline': 'ompl',
    }

    # Load OMPL planning config
    ompl_planning_yaml = load_yaml(package_name, 'config/ompl_planning.yaml')

    # Controllers config
    moveit_controllers_yaml = load_yaml(package_name, 'config/moveit_controllers.yaml')
    moveit_controllers = {
        'moveit_simple_controller_manager': moveit_controllers_yaml['moveit_simple_controller_manager'],
        'moveit_controller_manager': moveit_controllers_yaml['moveit_controller_manager']
    }

    # Trajectory execution and planning scene monitor config
    trajectory_execution = {
        'moveit_manage_controllers': True,
        'trajectory_execution.allowed_execution_duration_scaling': 1.2,
        'trajectory_execution.allowed_goal_duration_margin': 0.5,
        'trajectory_execution.allowed_start_tolerance': 0.01,
    }

    planning_scene_monitor_parameters = {
        'publish_planning_scene': True,
        'publish_geometry_updates': True,
        'publish_state_updates': True,
        'publish_transforms_updates': True,
        'publish_robot_description': True,
        'publish_robot_description_semantic': True,
    }

    # Move group node
    move_group_node = Node(
        package='moveit_ros_move_group',
        executable='move_group',
        output='screen',
        parameters=[
            robot_description,
            robot_description_semantic,
            robot_description_kinematics,
            robot_description_planning,
            planning_pipelines_config,
            {'ompl': ompl_planning_yaml},
            trajectory_execution,
            moveit_controllers,
            planning_scene_monitor_parameters,
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
