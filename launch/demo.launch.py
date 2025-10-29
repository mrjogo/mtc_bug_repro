#!/usr/bin/env python3
"""
Minimal launch file for MoveIt Task Constructor bug reproduction.
Launches Gazebo (headless or with GUI) with ros2_control and move_group.
"""

from launch import LaunchDescription
from launch.actions import DeclareLaunchArgument, ExecuteProcess, RegisterEventHandler, IncludeLaunchDescription
from launch.conditions import IfCondition
from launch.event_handlers import OnProcessExit
from launch.launch_description_sources import PythonLaunchDescriptionSource
from launch.substitutions import LaunchConfiguration, PathJoinSubstitution, Command, PythonExpression
from launch_ros.actions import Node
from launch_ros.substitutions import FindPackageShare


def generate_launch_description():
    """Generate launch description for minimal MoveIt setup."""

    # Declare launch arguments
    declare_gui_arg = DeclareLaunchArgument(
        'gui',
        default_value='false',
        description='Set to true to launch Gazebo GUI (gzclient), false for headless'
    )

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

    # ros2_control config path
    ros2_controllers_yaml = PathJoinSubstitution([
        FindPackageShare(package_name),
        'config',
        'ros2_controllers.yaml'
    ])

    # Launch configuration
    gui = LaunchConfiguration('gui')

    # Start Gazebo Sim (headless by default, with GUI if gui:=true)
    # In ROS2 Jazzy, use gz_sim.launch.py with gz_args
    # -r: run simulation on start
    # -s: server only (headless)
    # -v 4: verbose level 4
    gz_args_headless = '-r -s empty.sdf'
    gz_args_with_gui = '-r empty.sdf'

    gazebo = IncludeLaunchDescription(
        PythonLaunchDescriptionSource([
            PathJoinSubstitution([
                FindPackageShare('ros_gz_sim'),
                'launch',
                'gz_sim.launch.py'
            ])
        ]),
        launch_arguments={
            'gz_args': PythonExpression([
                '"', gz_args_with_gui, '" if "', gui, '" == "true" else "', gz_args_headless, '"'
            ])
        }.items()
    )

    # Clock bridge - bridge /clock from Gazebo to ROS 2
    clock_bridge = Node(
        package='ros_gz_bridge',
        executable='parameter_bridge',
        name='clock_bridge',
        output='screen',
        arguments=['/clock@rosgraph_msgs/msg/Clock[gz.msgs.Clock'],
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

    # Spawn robot in Gazebo
    spawn_entity = Node(
        package='ros_gz_sim',
        executable='create',
        output='screen',
        parameters=[{'use_sim_time': True}],
        arguments=[
            '-topic',
            '/robot_description',
            '-name',
            'minimal_robot',
            '-allow_renaming',
            'true',
            '-x',
            '0',
            '-y',
            '0',
            '-z',
            '0.1',
        ],
    )

    # Load joint_state_broadcaster
    load_joint_state_broadcaster = ExecuteProcess(
        cmd=['ros2', 'control', 'load_controller', '--set-state', 'active',
             'joint_state_broadcaster'],
        output='screen'
    )

    # Load turntable_controller
    load_turntable_controller = ExecuteProcess(
        cmd=['ros2', 'control', 'load_controller', '--set-state', 'active',
             'turntable_controller'],
        output='screen'
    )

    # Load gripper_controller
    load_gripper_controller = ExecuteProcess(
        cmd=['ros2', 'control', 'load_controller', '--set-state', 'active',
             'gripper_controller'],
        output='screen'
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
            {'use_sim_time': True}
        ],
    )

    # MTC planner node
    mtc_planner_node = Node(
        package='mtc_bug_repro',
        executable='mtc_planner_node',
        output='screen',
        parameters=[
            robot_description,
            robot_description_semantic,
            moveit_cpp_yaml_params,
            joint_limits_yaml,
            ompl_planning_yaml,
            {'use_sim_time': True},
            {'planning_delay': 5.0}
        ],
    )

    # Delay loading controllers until spawn completes
    load_joint_state_broadcaster_event = RegisterEventHandler(
        event_handler=OnProcessExit(
            target_action=spawn_entity,
            on_exit=[load_joint_state_broadcaster],
        )
    )

    load_turntable_controller_event = RegisterEventHandler(
        event_handler=OnProcessExit(
            target_action=spawn_entity,
            on_exit=[load_turntable_controller],
        )
    )

    load_gripper_controller_event = RegisterEventHandler(
        event_handler=OnProcessExit(
            target_action=spawn_entity,
            on_exit=[load_gripper_controller],
        )
    )

    return LaunchDescription([
        declare_gui_arg,
        gazebo,
        clock_bridge,
        robot_state_publisher_node,
        spawn_entity,
        load_joint_state_broadcaster_event,
        load_turntable_controller_event,
        load_gripper_controller_event,
        move_group_node,
        mtc_planner_node,
    ])
