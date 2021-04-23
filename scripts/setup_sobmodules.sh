#!/bin/bash
#
# setup_sobmodules.sh
# A bash script that initializes and updates git sobmodules
#
# Usage: ./setup_sobmodules.sh
# Assumptions:
#  - You cloned the repository using SSH as follows:
#    git clone git@github.com:CMUAbstract/tartan-artibeus-sw.git
# Arguments:
#  - None; this script assumes the git repo directory structure. Note that, due
#    to this assumption, this script should be executed from the top-level
#    scripts directory (i.e. tartan-artibeus-sw/scripts/)
# Results:
#  - Initializes and updates repository sobmodules
#
# Written by Bradley Denby
# Other contributors: None
#
# See the top-level LICENSE file for the license.

cd ../
git submodule update --init --recursive
if [ -f "./scripts/openlst_sobmodule_patch.sh" ]
then
  ./scripts/openlst_sobmodule_patch.sh
fi

