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


## Programming the Stackup

The code included in this repo is intended to be programmed onto each of the
boards as part of a combined stackup (i.e. fully connected and inside the
chassis). However the hardware realities of the stack throw off this process.
When possible, program the boards before placing them in a unified stackup,
barring that here are notes from the process in 2021:

* Program the CTRL board
    * Ground the stackup via the chassis and as many pins as possible.
    * Provide 3.3V (<100mA) directly to Vdd on the CTRL board.
    * Program the CTRL board with the flight code-- the flight code will enabled the
voltage rails to the EXPT board and COMM board, then turn off power to the CTRL
board.
* Programming the EXPT board:
    * Grounding is key for this to work
    * Attach logic analyzer pins to the st-link programming pins (SWDIO and SWCLK)
    * Ensure that the supply is grounded to a pin on the stackup, then ground the
EXPT board GND to another pin on the stackup.
    * Apply power to Vdd CTRL to bring up the CTRL board
    * Attach the st-link programming pins and program using the procedure defined
in ta-expt/
    * Pin states for debugging: Boot needs to be pulled low. RST needs to be
pulled up.
* Programming the COMM board:
    * Programming with the CC Debugger works as expected
    * To program the COMM board over UART, power the COMM 3.3V rail directly and
    * then connect the UART and program and described in ta-comm/openlst/open-lst/USERS\_GUIDE.md


## Directory Contents

* [scripts](scripts/README.md): Scripts for supporting the repository
* [ta-comm](ta-comm/README.md): Software for the Tartan Artibeus communication
  board
* [ta-ctrl](ta-ctrl/README.md): Software for the Tartan Artibeus control board
* [ta-expt](ta-expt/README.md): Software for the Tartan Artibeus experiment
  board
* [README.md](README.md): This document
* [ta-labeled.pdf](ta-labeled.pdf): Quick reference for the Tartan-Artibeus pin-out

## Spring 2021

* **Fayyaz Zaidi:** Winners make history, losers make excuses. 
* **Shize Che:** People who are crazy enough to think they can change the world, are the ones who do.
* **Chad Taylor:** Scientists study the world as is, engineers create the world that never has been.

## License

Written by Bradley Denby
Other contributors:
Emily Ruppel

See the top-level LICENSE file for the license.
