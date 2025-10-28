#!/bin/bash
set -e

echo "Setting up development environment for current user..."

# Get current user info
USERNAME=$(whoami)
USER_HOME=$(eval echo ~$USERNAME)

# Set up the ROS environment if not already in bashrc
if ! grep -q "source /opt/ros/jazzy/setup.bash" "$USER_HOME/.bashrc"; then
  echo "Adding ROS setup to .bashrc"
  echo "source /opt/ros/jazzy/setup.bash" >> "$USER_HOME/.bashrc"
fi

# Create VSCode settings directory and configure Claude Code
VSCODE_SETTINGS_DIR="$USER_HOME/.vscode-server/data/Machine"
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
  "claude-code.claudeProcessWrapper": "/workspaces/dev_ws/src/mtc_bug_repro/.devcontainer/claude-dangerous.sh"
}
EOF
fi

# Up arrow bash history search
cat > ~/.inputrc << EOF
# Respect default shortcuts.
\$include /etc/inputrc

## arrow up
"\e[A":history-search-backward
## arrow down
"\e[B":history-search-forward
EOF

# Change ownership of the dev_ws directory to the current user
sudo chown -R "$USERNAME":"$USERNAME" /workspaces/dev_ws

echo "Setup complete for user: $USERNAME"
