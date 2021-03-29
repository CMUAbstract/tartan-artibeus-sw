#!/bin/bash
#
# openlst_sobmodule_patch.sh
# A bash script that patches OpenLST to work with git sobmodules
#
# Usage: cd../ && ./scripts/setup_sobmodules.sh
# Assumptions:
#  - This script should be called by the setup_sobmodules.sh script
# Arguments:
#  - None; this script assumes the git repo directory structure
# Results:
#  - Patches the OpenLST sobmodule to avoid a compilation error
#
# Written by Bradley Denby
# Other contributors: None
#
# See the top-level LICENSE file for the license.

sed -i -e "s,GIT_REV = \$(shell git describe --always --dirty),GIT_REV = \"sobmodule-fix\"," ta-comm/openlst/open-lst/Build.mk
sed -i -e "s,GIT_REV_HEX = 0x\$(shell git rev-parse --short=8 HEAD),GIT_REV_HEX = 0xsobmodule-fix," ta-comm/openlst/open-lst/Build.mk
