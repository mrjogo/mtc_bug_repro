cmake_minimum_required(VERSION 3.22)
project(mtc_bug_repro)

# Default to C++17
if(NOT CMAKE_CXX_STANDARD)
  set(CMAKE_CXX_STANDARD 17)
  set(CMAKE_CXX_STANDARD_REQUIRED ON)
endif()

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# Find dependencies
find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)

# MoveIt2 dependencies
find_package(moveit_core REQUIRED)
find_package(moveit_ros_planning_interface REQUIRED)

# MoveIt Task Constructor dependencies
find_package(moveit_task_constructor_core REQUIRED)
find_package(moveit_task_constructor_msgs REQUIRED)

# Collect all dependencies for easier linking
set(THIS_PACKAGE_DEPENDENCIES
  rclcpp
  moveit_core
  moveit_ros_planning_interface
  moveit_task_constructor_core
  moveit_task_constructor_msgs
)

# Include directories
include_directories(
  include
)

#############
## Build   ##
#############

# Build the PrepareStage library
add_library(mtc_stages
  src/prepare_stage.cpp
)
ament_target_dependencies(mtc_stages ${THIS_PACKAGE_DEPENDENCIES})

# Build the mtc_planner_node executable
add_executable(mtc_planner_node
  src/mtc_planner_node.cpp
)
ament_target_dependencies(mtc_planner_node ${THIS_PACKAGE_DEPENDENCIES})
target_link_libraries(mtc_planner_node mtc_stages)

#############
## Install ##
#############

# Install executables
install(TARGETS
  mtc_planner_node
  DESTINATION lib/${PROJECT_NAME}
)

# Install libraries
install(TARGETS
  mtc_stages
  ARCHIVE DESTINATION lib
  LIBRARY DESTINATION lib
  RUNTIME DESTINATION bin
)

# Install header files
install(DIRECTORY include/
  DESTINATION include
)

# Install launch files
install(DIRECTORY launch
  DESTINATION share/${PROJECT_NAME}
)

# Install config files
install(DIRECTORY config
  DESTINATION share/${PROJECT_NAME}
)

# Install URDF files
install(DIRECTORY urdf
  DESTINATION share/${PROJECT_NAME}
)

#############
## Testing ##
#############

if(BUILD_TESTING)
  find_package(ament_lint_auto REQUIRED)
  # the following line skips the linter which checks for copyrights
  # comment the line when a copyright and license is added to all source files
  set(ament_cmake_copyright_FOUND TRUE)
  # the following line skips cpplint (only works in a git repo)
  # comment the line when this package is in a git repo and when
  # a copyright and license is added to all source files
  set(ament_cmake_cpplint_FOUND TRUE)
  ament_lint_auto_find_test_dependencies()
endif()

#############
## Export  ##
#############

# Export dependencies
ament_export_dependencies(
  ${THIS_PACKAGE_DEPENDENCIES}
)

# Export include directories
ament_export_include_directories(
  include
)

# Export libraries
ament_export_libraries(
  mtc_stages
)

# Export targets (modern CMake approach)
# ament_export_targets(
#   export_${PROJECT_NAME}
# )

ament_package()
