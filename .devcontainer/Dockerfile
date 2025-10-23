FROM ros:jazzy-ros-base

# Install Gazebo
RUN apt-get update && apt-get install -y \
    ros-jazzy-desktop-full \
    && rm -rf /var/lib/apt/lists/*

# Create a non-root user with the same UID/GID as the host user
ARG USERNAME=ros
ARG USER_UID=1000
ARG USER_GID=1000

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

RUN mkdir -p /workspaces/dev_ws/ && chown $USER_UID:$USER_GID /workspaces/dev_ws/

# Set the user and workspace
USER $USERNAME
WORKDIR /workspaces/dev_ws/src/mtc_bug_repro
