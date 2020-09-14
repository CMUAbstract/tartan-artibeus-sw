#!/bin/bash
#
# setup_dependencies.sh
# A bash script that sets up dependencies
#
# Usage: ./setup_dependencies.sh
# Arguments:
#  - None; this script assumes the git repo directory structure. Note that, due
#    to this assumption, this script should be executed from the containing
#    scripts directory
# Results:
#  - Sets up dependencies for the ta-comm software
#
# Written by Bradley Denby
# Other contributors: None
#
# See the top-level LICENSE file for the license.

# sudo apt message
echo ""
echo "Run the following manually first (see top-level README.md):"
echo "  $ echo \"\" | sudo tee -a /etc/apt/sources.list > /dev/null"
echo "  $ echo \"deb [arch=amd64] https://download.virtualbox.org/virtualbox/debian $(lsb_release --short --codename) contrib\" | sudo tee -a /etc/apt/sources.list > /dev/null"
echo "  $ wget -q https://www.virtualbox.org/download/oracle_vbox_2016.asc -O- | sudo apt-key add -"
echo "  $ sudo apt update"
echo "  $ sudo apt install virtualbox-6.1"
echo "  $ wget https://download.virtualbox.org/virtualbox/6.1.14/Oracle_VM_VirtualBox_Extension_Pack-6.1.14.vbox-extpack"
echo "  $ vboxmanage extpack install --replace --accept-license=sha256 Oracle_VM_VirtualBox_Extension_Pack-6.1.14.vbox-extpack"
echo "  $ wget https://releases.hashicorp.com/vagrant/2.2.10/vagrant_2.2.10_x86_64.deb"
echo "  $ sudo dpkg -i vagrant_2.2.10_x86_64.deb"
echo ""

# Submodules
cd ../../scripts/
./setup_sobmodules.sh

# Vagrant
echo ""
echo "Running Vagrant:"
echo "  $ cd openlst/"
echo "  $ vagrant up"
echo "  $ vagrant reload"
echo "Halting Vagrant:"
echo "  $ vagrant halt"
echo ""
