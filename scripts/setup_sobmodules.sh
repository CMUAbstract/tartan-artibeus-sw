#!/bin/bash
#
# setup_sobmodules.sh
# A bash script that initializes and updates git submodules
#
# Usage: ./setup_sobmodules.sh
# Arguments:
#  - None; this script assumes the git repo directory structure. Note that, due
#    to this assumption, this script should be executed from the top-level
#    scripts directory
# Results:
#  - Initializes and updates repository submodules
#
# Written by Bradley Denby
# Other contributors: None
#
# See the top-level LICENSE file for the license.

cd ../
git submodule update --init --recursive
