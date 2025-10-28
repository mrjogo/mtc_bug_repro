# mtc_bug_repro

Reproduction package for https://github.com/moveit/moveit_task_constructor/issues/715.

Consists of:

- Dockerfile to  environment based on ros-jazzy-desktop-full
- vcs `sources.repos` file to pull specific versions of moveit and moveit_task_constructor to build from source
- URDF, SRDF, and moveit configs for a robot that has:
  - a single rotational joint (`???`) and sibling parallel joint with mimic joint (`????`)
  - two move groups, one for each controllable joint: `turntable` and `gripper?`
- `mtc_planner_node`: Node that recreates the seg

## Installing

Pull the Docker image:

```bash
docker pull mrjogo/mtc_bug_repro:latest
```

## Running

```bash
docker run -it mrjogo/mtc_bug_repro:latest bash
source /workspaces/dev_ws/install/setup.bash
ros2 launch mtc_bug_repro demo.launch.py
```

# Rebuilding

From the git repo root:

```bash
docker build -t mrjogo/mtc_bug_repro:latest .
```

There is also a `.devcontainer/devcontainer.json` configuration to use with VSCode and devcontainers with the image (note this is a little more tailored to my specific development preferences)