#!/bin/bash
# Just use VS Code's bundled binary directly with dangerous permissions
exec "$1" --dangerously-skip-permissions "${@:2}"