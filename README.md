# Tartan Artibeus Software

This repository contains multiple directories, each of which contains software
for a Tartan Artibeus board.

**Current version**: 0.0.0

* This software uses [semantic versioning](http://semver.org).

**Dependencies**

```bash
# install ta-comm dependencies
echo "" | sudo tee -a /etc/apt/sources.list > /dev/null
echo "deb [arch=amd64] https://download.virtualbox.org/virtualbox/debian $(lsb_release --short --codename) contrib" | sudo tee -a /etc/apt/sources.list > /dev/null
wget -q https://www.virtualbox.org/download/oracle_vbox_2016.asc -O- | sudo apt-key add -
sudo apt update
sudo apt install virtualbox-6.1
wget https://download.virtualbox.org/virtualbox/6.1.14/Oracle_VM_VirtualBox_Extension_Pack-6.1.14.vbox-extpack
vboxmanage extpack install --replace --accept-license=sha256 Oracle_VM_VirtualBox_Extension_Pack-6.1.14.vbox-extpack
wget https://releases.hashicorp.com/vagrant/2.2.10/vagrant_2.2.10_x86_64.deb
sudo dpkg -i vagrant_2.2.10_x86_64.deb
# install ta-expt dependencies
sudo apt install build-essential cmake gcc libusb-1.0-0 libusb-1.0-0-dev libgtk-3-dev
sudo cp ta-expt/utilities/stlink/config/udev/rules.d/*.rules /etc/udev/rules.d/
# initialize and update submodules
cd $HOME/git-repos/tartan-artibeus-sw/scripts
./setup_sobmodules.sh
```

## Directory Contents

* [ta-comm](ta-comm/README.md): Software for the Tartan Artibeus communication
  board
* [ta-ctrl](ta-ctrl/README.md): Software for the Tartan Artibeus control board
* [ta-expt](ta-expt/README.md): Software for the Tartan Artibeus experiment
  board
* [README.md](README.md): This document

## License

License information pending
