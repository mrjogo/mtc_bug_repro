#include <mtc_bug_repro/stages/prepare_stage.h>
#include <moveit/task_constructor/stages/current_state.h>
#include <moveit/task_constructor/stages/move_to.h>
#include <moveit/task_constructor/container.h>
#include <moveit/task_constructor/solvers/joint_interpolation.h>

namespace mtc_bug_repro
{

PrepareStage::PrepareStage(const rclcpp::Node::SharedPtr& node, const std::string& name)
  : SerialContainer(name)
{
  // Create JointInterpolation planner for this stage
  auto interpolation_planner = std::make_shared<moveit::task_constructor::solvers::JointInterpolationPlanner>();

  // Add current state as the starting point
  {
    auto current_state = std::make_unique<moveit::task_constructor::stages::CurrentState>("current state");
    add(std::move(current_state));
  }

  // Create a Merger container to execute multiple moves in parallel
  {
    auto merger = std::make_unique<moveit::task_constructor::Merger>("move to initial positions");

    // Move gripper to open position
    {
      auto open_gripper = std::make_unique<moveit::task_constructor::stages::MoveTo>("open gripper", interpolation_planner);
      open_gripper->setGroup("gripper");
      open_gripper->setGoal("open");
      open_gripper->properties().configureInitFrom(moveit::task_constructor::Stage::PARENT);
      merger->insert(std::move(open_gripper));
    }

    // Move turntable to close position
    {
      auto move_turntable = std::make_unique<moveit::task_constructor::stages::MoveTo>("move turntable", interpolation_planner);
      move_turntable->setGroup("turntable");
      move_turntable->setGoal("close");
      move_turntable->properties().configureInitFrom(moveit::task_constructor::Stage::PARENT);
      merger->insert(std::move(move_turntable));
    }

    add(std::move(merger));
  }
}

}  // namespace mtc_bug_repro
