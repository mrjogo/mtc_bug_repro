# docker build -t mrjogo/mtc_bug_repro:latest .
FROM ros:jazzy-ros-base

# Install ros desktop full and remove moveit (will build from source)
RUN apt-get update && apt-get install -y ros-jazzy-desktop-full \
    && apt-get purge -y ros-jazzy-moveit-* \
    && rm -rf /var/lib/apt/lists/*

# Install and build dependencies
RUN rosdep update
RUN mkdir -p /workspaces/dev_ws/src/mtc_bug_repro
COPY ./package.xml /workspaces/dev_ws/src/mtc_bug_repro/package.xml
COPY ./sources.repos /tmp/sources.repos
RUN vcs import --input /tmp/sources.repos --recursive /workspaces/dev_ws/src
RUN . /opt/ros/jazzy/setup.sh \
  && sudo apt-get update \
  && PIP_BREAK_SYSTEM_PACKAGES=1 rosdep install -i -r -y --from-paths /workspaces/dev_ws/src \
  # Clean up rosdep cache
  && rm -rf /home/$USERNAME/.ros/rosdep
WORKDIR /workspaces/dev_ws
RUN . /opt/ros/jazzy/setup.sh \
  && colcon build --packages-up-to mtc_bug_repro --packages-skip mtc_bug_repro --parallel-workers 3 \
  && rm -rf /workspaces/dev_ws/build /workspaces/dev_ws/logs /workspaces/dev_ws/install/mtc_bug_repro /workspaces/dev_ws/src/mtc_bug_repro

# Build mtc_bug_repro
COPY . /workspaces/dev_ws/src/mtc_bug_repro
RUN . /workspaces/dev_ws/install/setup.sh \
  && colcon build --packages-select mtc_bug_repro

WORKDIR /workspaces/dev_ws/src/mtc_bug_repro
