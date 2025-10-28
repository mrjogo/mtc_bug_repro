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
  }

  bool setupAndPlanTask()
  {
    try {
      // Create a Task instance
      mtc::Task task("prepare_task");
      task.stages()->setName("Prepare Robot Task");

      // Load robot model
      task.loadRobotModel(shared_from_this());

      // Add the custom PrepareStage
      RCLCPP_INFO(this->get_logger(), "Adding PrepareStage to task");
      auto prepare_stage = std::make_unique<mtc_bug_repro::PrepareStage>(
        shared_from_this(), "prepare robot");
      task.add(std::move(prepare_stage));

      // Initialize task
      RCLCPP_INFO(this->get_logger(), "Initializing task...");
      try {
        task.init();
      } catch (const mtc::InitStageException& e) {
        RCLCPP_ERROR(this->get_logger(), "Task initialization failed: %s", e.what());
        return false;
      }

      // Plan the task (but do NOT execute it)
      RCLCPP_INFO(this->get_logger(), "Planning task...");
      if (!task.plan(5)) {
        RCLCPP_ERROR(this->get_logger(), "Task planning failed");
        return false;
      }

      // Print results
      RCLCPP_INFO(this->get_logger(), "Planning succeeded!");
      RCLCPP_INFO(this->get_logger(), "Number of solutions: %zu", task.solutions().size());

      // Print solution details
      size_t solution_idx = 1;
      for (const auto& solution : task.solutions()) {
        RCLCPP_INFO(this->get_logger(),
                    "Solution %zu: cost = %.3f", solution_idx++, solution->cost());
      }

      return true;

    } catch (const std::exception& e) {
      RCLCPP_ERROR(this->get_logger(), "Exception during task setup/planning: %s", e.what());
      return false;
    }
  }
};

int main(int argc, char** argv)
{
  // Initialize ROS2
  rclcpp::init(argc, argv);

  // Create node options
  rclcpp::NodeOptions node_options;
  node_options.automatically_declare_parameters_from_overrides(true);

  // Create the node
  auto node = std::make_shared<MtcPlannerNode>(node_options);

  // Create a separate thread for spinning the node
  std::thread spinner([node]() {
    rclcpp::executors::MultiThreadedExecutor executor;
    executor.add_node(node);
    executor.spin();
  });

  // Give some time for the node to fully initialize
  std::this_thread::sleep_for(std::chrono::seconds(2));

  // Setup and plan the task
  RCLCPP_INFO(node->get_logger(), "Starting task planning...");
  bool success = node->setupAndPlanTask();

  if (success) {
    RCLCPP_INFO(node->get_logger(), "Task planning completed successfully");
  } else {
    RCLCPP_ERROR(node->get_logger(), "Task planning failed");
  }

  // Shutdown
  rclcpp::shutdown();
  spinner.join();

  return success ? 0 : 1;
}
