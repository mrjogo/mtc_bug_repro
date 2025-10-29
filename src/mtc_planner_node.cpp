#include <rclcpp/rclcpp.hpp>
#include <moveit/planning_scene/planning_scene.hpp>
#include <moveit/planning_scene_interface/planning_scene_interface.hpp>
#include <moveit/task_constructor/task.h>
#include <moveit/task_constructor/solvers.h>
#include <moveit/task_constructor/stages.h>
#include <moveit/move_group_interface/move_group_interface.h>
#include <mtc_bug_repro/prepare_stage.h>

namespace mtc = moveit::task_constructor;

class MtcPlannerNode : public rclcpp::Node
{
public:
  MtcPlannerNode(const rclcpp::NodeOptions& options = rclcpp::NodeOptions())
    : Node("mtc_planner_node", options)
  {
    RCLCPP_INFO(this->get_logger(), "Initializing MTC Planner Node");

    // Declare and get the planning delay parameter (in seconds)
    this->declare_parameter("planning_delay", 2.0);
    double planning_delay = this->get_parameter("planning_delay").as_double();

    // Declare and get boolean parameters for initial positions
    this->declare_parameter("turntable_at_target", false);
    this->declare_parameter("gripper_at_target", false);
    bool turntable_at_target = this->get_parameter("turntable_at_target").as_bool();
    bool gripper_at_target = this->get_parameter("gripper_at_target").as_bool();

    // Translate boolean parameters to joint positions
    // turntable_at_target: true -> 0.0 (close position), false -> 0.26 (start at open)
    // gripper_at_target: true -> open position (0.04), false -> 0.0 (closed)
    // Note: gripper_right_finger is a mimic joint, so we only command gripper_left_finger
    turntable_position_ = turntable_at_target ? 0.0 : 0.26;  // Target is "close" = 0.0, start is "open" = 0.26
    gripper_left_position_ = gripper_at_target ? 0.04 : 0.0;

    RCLCPP_INFO(this->get_logger(), "Turntable at target: %s -> position: %.3f rad",
                turntable_at_target ? "true" : "false", turntable_position_);
    RCLCPP_INFO(this->get_logger(), "Gripper at target: %s -> left finger position: %.3f m",
                gripper_at_target ? "true" : "false", gripper_left_position_);
    RCLCPP_INFO(this->get_logger(), "Planning will start in %.1f seconds", planning_delay);

    // Create a one-shot timer to trigger planning
    planning_timer_ = this->create_wall_timer(
      std::chrono::duration<double>(planning_delay),
      std::bind(&MtcPlannerNode::planningTimerCallback, this));
  }

private:
  void planningTimerCallback()
  {
    // Cancel the timer so it only fires once
    planning_timer_->cancel();

    RCLCPP_INFO(this->get_logger(), "Timer fired - moving joints to target positions...");

    // Move joints to target positions before executing task
    if (!moveJointsToTargetPositions()) {
      RCLCPP_ERROR(this->get_logger(), "Failed to move joints to target positions");
      return;
    }

    RCLCPP_INFO(this->get_logger(), "Joints moved successfully, now executing task...");

    // Execute the task
    executeTask();
  }

  bool moveJointsToTargetPositions()
  {
    try {
      // Create MoveGroupInterface for the planning group
      // Using "full_robot" group which includes both turntable and gripper
      auto move_group = std::make_shared<moveit::planning_interface::MoveGroupInterface>(
        shared_from_this(), "full_robot");

      RCLCPP_INFO(this->get_logger(), "Setting joint value targets...");

      // Set the target joint values
      // Note: gripper_right_finger_joint is a mimic joint, so we only set gripper_left_finger_joint
      std::map<std::string, double> target_joints;
      target_joints["turntable_joint"] = turntable_position_;
      target_joints["gripper_left_finger_joint"] = gripper_left_position_;

      move_group->setJointValueTarget(target_joints);

      RCLCPP_INFO(this->get_logger(), "Planning motion to target joint positions...");

      // Plan and execute the motion
      moveit::planning_interface::MoveGroupInterface::Plan plan;
      bool success = (move_group->plan(plan) == moveit::core::MoveItErrorCode::SUCCESS);

      if (success) {
        RCLCPP_INFO(this->get_logger(), "Plan successful, executing motion...");
        auto result = move_group->execute(plan);
        if (result == moveit::core::MoveItErrorCode::SUCCESS) {
          RCLCPP_INFO(this->get_logger(), "Motion execution completed successfully");
          return true;
        } else {
          RCLCPP_ERROR(this->get_logger(), "Motion execution failed");
          return false;
        }
      } else {
        RCLCPP_ERROR(this->get_logger(), "Failed to plan motion to target positions");
        return false;
      }
    } catch (const std::exception& e) {
      RCLCPP_ERROR(this->get_logger(), "Exception during joint movement: %s", e.what());
      return false;
    }
  }

  void executeTask()
  {
    RCLCPP_INFO(this->get_logger(), "Starting task planning...");

    try {
      // Create a Task instance
      auto root_container = std::make_unique<mtc_bug_repro::PrepareStage>(shared_from_this());
      auto task = std::make_shared<moveit::task_constructor::Task>("", true, std::move(root_container));
      task->loadRobotModel(shared_from_this());

      // Initialize task
      RCLCPP_INFO(this->get_logger(), "Initializing task...");
      try {
        task->init();
      } catch (const mtc::InitStageException& e) {
        RCLCPP_ERROR(this->get_logger(), "Task initialization failed: %s", e.what());
        return;
      }

      // Plan the task (but do NOT execute it)
      RCLCPP_INFO(this->get_logger(), "Planning task...");
      if (!task->plan(5)) {
        RCLCPP_ERROR(this->get_logger(), "Task planning failed");
        return;
      }

      // Print results
      RCLCPP_INFO(this->get_logger(), "Planning succeeded!");

      // Execute the first solution
      if (!task->solutions().empty()) {
        RCLCPP_INFO(this->get_logger(), "Executing first solution...");
        task->execute(*task->solutions().front());
        RCLCPP_INFO(this->get_logger(), "Execution completed");
      } else {
        RCLCPP_WARN(this->get_logger(), "No solutions available to execute");
      }
    } catch (const std::exception& e) {
      RCLCPP_ERROR(this->get_logger(), "Exception during task setup/planning: %s", e.what());
    }
  }

  rclcpp::TimerBase::SharedPtr planning_timer_;
  double turntable_position_;
  double gripper_left_position_;
};

int main(int argc, char** argv)
{
  // Initialize ROS2
  rclcpp::init(argc, argv);

  // Create the node
  auto node = std::make_shared<MtcPlannerNode>();

  // Spin the executor (planning will happen via the timer)
  rclcpp::executors::MultiThreadedExecutor executor;
  executor.add_node(node);
  executor.spin();

  // Shutdown
  rclcpp::shutdown();

  return 0;
}
