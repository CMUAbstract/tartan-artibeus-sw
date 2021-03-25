# Tartan Artibeus Experiment Board

Tartan Artibeus experiment board (`ta-expt`)

See the top-level [README](../README.md) for dependency setup prerequisites.

## Directory Contents

* [scripts](scripts/README.md): `ta-expt` setup and development scripts
* [software](software/README.md): `ta-expt` example programs
* [utilities](utilities/README.md): `ta-expt` support utilities
* [README.md](README.md): This document

## Useful development tools

```bash
sudo usermod -aG dialout $USER
# Restart the OS
sudo apt install minicom
sudo chmod go+w /etc/minicom/
minicom -s
# Serial port setup
#   Serial Device: /dev/ttyUSB0
#   Bps/Par/Bits:  115200 8N1
#   Hardware Flow Control: No
#   Software Flow Control: No
#   Press Enter to accept
# Save setup as...
#   ta-expt
# Test that the configuration was successful:
minicom ta-expt
# Local echo (shows the chars you send over serial):
#   Ctrl-a e
# Exit minicom:
#   Ctrl-a x
# Specify a file to log all serial traffic:
minicom -C logfile.txt ta-expt
# Open minicom in hex mode (useful for examining RX'd TAOLST protocol)
minicom -H ta-expt
# Combine both:
minicom -C logfile.txt -H ta-expt
```

## License

Written by Bradley Denby  
Other contributors: None

See the top-level LICENSE file for the license.
