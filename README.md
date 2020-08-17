# Tartan Artibeus Software

This repository contains multiple directories, each of which contains software
for a Tartan Artibeus board.

**Current version**: 0.0.0

* This software uses [semantic versioning](http://semver.org).

**Dependencies**

```bash
# install ta-expt dependencies
sudo apt install build-essential cmake gcc libusb-1.0 libusb-1.0-0-dev libgtk-3-dev
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
