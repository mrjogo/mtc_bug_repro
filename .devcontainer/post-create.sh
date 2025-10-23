#!/bin/bash
set -e

# Create a non-root user with the same UID/GID as the host user
if [ -z "${USERNAME}" ]; then
  echo "ERROR: USERNAME environment variable is not set"
  exit 1
fi

if [ -z "${USER_UID}" ]; then
  echo "ERROR: USER_UID environment variable is not set"
  exit 1
fi

if [ -z "${USER_GID}" ]; then
  echo "ERROR: USER_GID environment variable is not set"
  exit 1
fi

echo "Creating user: $USERNAME (UID: $USER_UID, GID: $USER_GID)"

# Delete user if USER_UID already exists
EXISTING_USER=$(getent passwd "$USER_UID" | cut -d: -f1)
if [ -n "$EXISTING_USER" ]; then
  echo "User with UID $USER_UID already exists ($EXISTING_USER), deleting..."
  userdel -r "$EXISTING_USER" 2>/dev/null || true
fi

# Create the group if it doesn't exist
groupadd --gid $USER_GID $USERNAME 2>/dev/null || true

# Create the user with matching UID/GID and bash as default shell
useradd --uid $USER_UID --gid $USER_GID -m -s /bin/bash $USERNAME 2>/dev/null || true

# Set up sudo permissions
echo "$USERNAME ALL=(root) NOPASSWD:ALL" > /etc/sudoers.d/$USERNAME
chmod 0440 /etc/sudoers.d/$USERNAME

# Set up the same environment as root user
echo "source /opt/ros/jazzy/setup.bash" >> /home/$USERNAME/.bashrc
cat /root/.bashrc >> /home/$USERNAME/.bashrc
chown $USERNAME:$USER_GID /home/$USERNAME/.bashrc

# Set ownership of the workspace
chown -R $USER_UID:$USER_GID /workspaces/dev_ws/

# Create VSCode settings directory and configure Claude Code
VSCODE_SETTINGS_DIR="/home/$USERNAME/.vscode-server/data/Machine"
mkdir -p "$VSCODE_SETTINGS_DIR"

# Create or update settings.json with Claude Code configuration
SETTINGS_FILE="$VSCODE_SETTINGS_DIR/settings.json"
if [ -f "$SETTINGS_FILE" ]; then
  echo "VSCode settings file exists, appending Claude Code configuration"
  # Remove the closing brace, add the setting, and close again
  sed -i '$ d' "$SETTINGS_FILE"
  echo '  ,"claude-code.claudeProcessWrapper": "/workspaces/dev_ws/src/mtc_bug_repro/claude-dangerous.sh"' >> "$SETTINGS_FILE"
  echo '}' >> "$SETTINGS_FILE"
else
  echo "Creating new VSCode settings file with Claude Code configuration"
  cat > "$SETTINGS_FILE" <<EOF
{
  "claude-code.claudeProcessWrapper": "/workspaces/dev_ws/src/mtc_bug_repro/claude-dangerous.sh"
}
EOF
fi

chown -R $USER_UID:$USER_GID "/home/$USERNAME/.vscode-server"

echo "User setup complete!"
