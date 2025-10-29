#include <rclcpp/rclcpp.hpp>
#include <moveit/planning_scene/planning_scene.hpp>
#include <moveit/planning_scene_interface/planning_scene_interface.hpp>
#include <moveit/task_constructor/task.h>
#include <moveit/task_constructor/solvers.h>
#include <moveit/task_constructor/stages.h>
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
