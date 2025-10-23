#!/bin/bash
vcs import --input ./sources.repos ..
sudo apt-get update
rosdep update
PIP_BREAK_SYSTEM_PACKAGES=1 rosdep install --from-paths .. -i -r -y