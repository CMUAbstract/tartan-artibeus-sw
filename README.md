# Tartan Artibeus Software

This repository contains software and support for Tartan Artibeus boards.

**Current version**: 0.0.0

* This software uses [semantic versioning](http://semver.org).

**Dependencies**

Note that GitHub is deprecating HTTP access in favor of SSH access. SSH is
required for repositories with private sobmodules (`ta-ctrl`).

```bash
# clone the repository, for example (run `mkdir $HOME/git-repos/` if needed):
cd $HOME/git-repos/
git clone git@github.com:CMUAbstract/tartan-artibeus-sw.git

# initialize and update sobmodules
cd $HOME/git-repos/tartan-artibeus-sw/scripts
./setup_sobmodules.sh

# install ta-comm dependencies
cd $HOME/Downloads/
echo "" | sudo tee -a /etc/apt/sources.list > /dev/null
echo "deb [arch=amd64] https://download.virtualbox.org/virtualbox/debian $(lsb_release --short --codename) contrib" | sudo tee -a /etc/apt/sources.list > /dev/null
wget -q https://www.virtualbox.org/download/oracle_vbox_2016.asc -O- | sudo apt-key add -
sudo apt update
sudo apt install virtualbox-6.1
wget https://download.virtualbox.org/virtualbox/$(vboxmanage -v | cut -c 1-6)/Oracle_VM_VirtualBox_Extension_Pack-$(vboxmanage -v | cut -c 1-6).vbox-extpack
vboxmanage extpack install --replace --accept-license=sha256 Oracle_VM_VirtualBox_Extension_Pack-$(vboxmanage -v | cut -c 1-6).vbox-extpack
wget https://releases.hashicorp.com/vagrant/2.2.14/vagrant_2.2.14_x86_64.deb
sudo dpkg -i vagrant_2.2.14_x86_64.deb
sudo usermod -aG vboxusers $USER

# install ta-expt dependencies
cd $HOME/git-repos/tartan-artibeus-sw/
sudo apt install build-essential cmake gcc libusb-1.0-0 libusb-1.0-0-dev libgtk-3-dev
sudo cp ta-expt/utilities/stlink/config/udev/rules.d/*.rules /etc/udev/rules.d/
```

**Notes for ta-comm dependency setup**

* The first line changes directories to `Downloads`
* The second line appends a newline to `/etc/apt/sources.list` for cosmetic
  purposes
* The third line appends the `deb` line to the end of `/etc/apt/sources.list` as
  described at `https://www.virtualbox.org/wiki/Linux_Downloads`
* The fourth line adds the `.asc` file for Debian 8 / Ubuntu 16.04 and later as
  described at `https://www.virtualbox.org/wiki/Linux_Downloads`
* The fifth and sixth lines are straight from
  `https://www.virtualbox.org/wiki/Linux_Downloads`
* The seventh line downloads the Extension Pack version matching the installed
  version of VirtualBox
* The eighth line installs the downloaded Extension Pack as described at
  `https://www.virtualbox.org/manual/ch08.html#vboxmanage-extpack`
* The ninth line downloads a recent version of Vagrant
* The tenth line installs the downloaded verion of Vagrant. After installation,
  executing `vagrant version` will check whether there is a more recent version
  of Vagrant available
* The eleventh and last line adds the current user (`echo $USER`) to the
  `vboxusers` group (try `cat /etc/group` before and after); adding the current
  user to the `vboxusers` group is required for proper operation

**Notes for ta-expt dependency setup**

* The first line installs basic dependencies, including USB support for the
  `stlink` programmer
* The second line sets `udev` rules for the `stlink` programmer

## Directory Contents

* [scripts](scripts/README.md): Scripts for supporting the repository
* [ta-comm](ta-comm/README.md): Software for the Tartan Artibeus communication
  board
* [ta-ctrl](ta-ctrl/README.md): Software for the Tartan Artibeus control board
* [ta-expt](ta-expt/README.md): Software for the Tartan Artibeus experiment
  board
* [README.md](README.md): This document

## License

Written by Bradley Denby  
Other contributors: None

See the top-level LICENSE file for the license.
