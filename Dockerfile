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
  && colcon build --packages-up-to mtc_bug_repro --packages-skip mtc_bug_repro --parallel-workers 3
  && rm -rf /workspaces/dev_ws/build /workspaces/dev_ws/logs /workspaces/dev_ws/install/mtc_bug_repro /workspaces/dev_ws/src/mtc_bug_repro

# Build mtc_bug_repro
COPY . /workspaces/dev_ws/src/mtc_bug_repro
RUN . /workspaces/dev_ws/install/setup.bash \
  && colcon build --packages-select mtc_bug_repro

# Create a non-root user with the same UID/GID as the host user
ARG USERNAME=ros
ARG USER_UID=501
ARG USER_GID=20

# Create the user with matching UID/GID and bash as default shell
RUN groupadd --gid $USER_GID $USERNAME || true \
    && useradd --uid $USER_UID --gid $USER_GID -m -s /bin/bash $USERNAME \
    && echo $USERNAME ALL=\(root\) NOPASSWD:ALL > /etc/sudoers.d/$USERNAME \
    && chmod 0440 /etc/sudoers.d/$USERNAME

# Set up the same environment as root user
RUN echo "source /opt/ros/jazzy/setup.bash" >> /home/$USERNAME/.bashrc \
    && cp /root/.bashrc /home/$USERNAME/.bashrc.backup \
    && cat /root/.bashrc >> /home/$USERNAME/.bashrc \
    && chown $USERNAME:$USER_GID /home/$USERNAME/.bashrc /home/$USERNAME/.bashrc.backup

RUN chown -R $USER_UID:$USER_GID /workspaces/dev_ws/

# Set the user and workspace
USER $USERNAME
WORKDIR /workspaces/dev_ws/src/mtc_bug_repro
