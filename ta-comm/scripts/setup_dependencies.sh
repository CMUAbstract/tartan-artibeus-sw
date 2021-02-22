#!/bin/bash
#
# setup_dependencies.sh
# A bash script that sets up dependencies
#
# Usage: ./setup_dependencies.sh
# Assumptions:
#  - The ta-comm prerequisites described in the top-level README have been
#    completed
# Arguments:
#  - None; this script assumes the git repo directory structure. Note that, due
#    to this assumption, this script should be executed from the containing
#    scripts directory (i.e. tartan-artibeus-sw/ta-comm/scripts/)
# Results:
#  - Sets up dependencies for the ta-comm software
#
# Written by Bradley Denby
# Other contributors: None
#
# See the top-level LICENSE file for the license.

# print prerequisite message
echo ""
echo "IMPORTANT NOTE: manually perform prerequisites in top-level README first"
echo ""

# sobmodules
cd ../../scripts/
./setup_sobmodules.sh

# vagrant
echo ""
echo "Running Vagrant:"
echo "  $ cd openlst/"
echo "  $ vagrant up"
echo "  $ vagrant reload"
echo "Halting Vagrant:"
echo "  $ vagrant halt"
echo ""
