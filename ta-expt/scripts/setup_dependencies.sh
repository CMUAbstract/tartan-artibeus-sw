#!/bin/bash
#
# setup_dependencies.sh
# A bash script that sets up dependencies
#
# Usage: ./setup_dependencies.sh
# Arguments:
#  - None; this script assumes the git repo directory structure. Note that, due
#    to this assumption, this script should be executed from the top-level
#    scripts directory
# Results:
#  - Sets up dependencies for the ta-expt software
#
# Written by Bradley Denby
# Other contributors: None
#
# See the top-level LICENSE file for the license.

# sudo apt message
echo ""
echo "Run the following manually first (see top-level README.md):"
echo "  $ sudo apt install build-essential cmake gcc libusb-1.0-0 libusb-1.0-0-dev libgtk-3-dev"
echo "  $ sudo cp ta-expt/utilities/stlink/config/udev/rules.d/*.rules /etc/udev/rules.d/"
echo ""

# Submodules
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
