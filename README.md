# mtc_bug_repro

Reproduction package for https://github.com/moveit/moveit_task_constructor/issues/715.

Consists of:

- Docker container to create ros-jazzy-desktop-full environment WITHOUT moveit packages installed
- Installation script that uses vcs to pull specific versions of moveit and moveit_task_constructor
- URDF, SRDF, and moveit configs for a robot that has:
  - a single rotational joint (`???`) and sibling parallel joint with mimic joint (`????`)
  - two move groups, one for each controllable joint: `turntable` and `gripper?`
- `mtc_planner_node`: Node that recreates the segfault bug using 

## Building

```bash
git clone https://github.com/mrjogo/mtc_bug_repro.git
cd mtc_bug_repro
devcontainer build --image-name mtc_bug_repro:latest # --config .devcontainer/build-only/devcontainer.json --workspace-folder .
docker run -it --net=host mtc_bug_repro:latest bash
# Now within the docker container
cd /workspaces/dev_ws/src/mtc_bug_repro
./install.sh
cd /workspaces/dev_ws
colcon build --packages-up-to mtc_bug_repro --parallel-workers 3
```

## Running

```bash
source ~/workspaces/dev_ws/install/setup.bash
ros2 launch mtc_bug_repro demo.launch.py
```
