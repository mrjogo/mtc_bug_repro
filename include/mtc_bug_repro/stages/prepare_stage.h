#pragma once

#include <rclcpp/rclcpp.hpp>
#include <moveit/task_constructor/container.h>

namespace mtc_bug_repro
{

/**
 * @brief Custom MTC stage that prepares the robot by setting initial state
 * and moving gripper and turntable to their initial positions
 */
class PrepareStage : public moveit::task_constructor::SerialContainer
{
public:
  /**
   * @brief Constructor for PrepareStage
   * @param node ROS2 node shared pointer
   * @param name Name of the stage
   */
  PrepareStage(const rclcpp::Node::SharedPtr& node, const std::string& name);

  virtual ~PrepareStage() = default;
};

}  // namespace mtc_bug_repro
