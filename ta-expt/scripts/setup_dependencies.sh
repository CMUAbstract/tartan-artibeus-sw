#!/bin/bash
#
# setup_dependencies.sh
# A bash script that sets up dependencies
#
# Usage: ./setup_dependencies.sh
# Assumptions:
#  - The ta-expt prerequisites described in the top-level README have been
#    completed
# Arguments:
#  - None; this script assumes the git repo directory structure. Note that, due
#    to this assumption, this script should be executed from the containing
#    scripts directory (i.e. tartan-artibeus-sw/ta-expt/scripts/)
# Results:
#  - Sets up dependencies for the ta-expt software
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

# stlink
cd ../ta-expt/utilities/stlink/
make clean
make release

# GNU Arm Embedded Toolchain
cd ../
wget https://developer.arm.com/-/media/Files/downloads/gnu-rm/9-2020q2/gcc-arm-none-eabi-9-2020-q2-update-x86_64-linux.tar.bz2
tar xjf gcc-arm-none-eabi-9-2020-q2-update-x86_64-linux.tar.bz2
echo "GNU Arm Embedded Toolchain ready"
echo "Don't forget to source the sourcefile:"
echo "  $ source sourcefile.txt"
echo "Be sure to source from the ta-expt/scripts directory"
cd ../scripts/
source sourcefile.txt

# libopencm3
cd ../software/libopencm3/
make
