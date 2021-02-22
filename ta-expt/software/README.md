# Tartan Artibeus Experiment Board Software

Tartan Artibeus experiment board software

**Usage**

Change directories to one of the software folders, e.g. `blink`, and execute
`make`. The Makefile includes the rules.mk file, as well as libopencm3 and any
common ASM or C files. Be sure to source `sourcefile.txt` first.

```bash
# Example
cd $HOME/git-repos/tartan-artibeus-sw/ta-expt/scripts/
source sourcefile.txt
cd $HOME/git-repos/tartan-artibeus-sw/ta-expt/software/blink/
make
```

See the README in each program directory for details.

## Directory Contents

* [blink](blink/README.md): Makes the Tartan Artibeus EXPT board LEDs blink
* [blink-rtos](blink-rtos/README.md): Blinks `ta-expt` LEDs using FreeRTOS
* [bootloader](bootloader/README.md): Basic bootloader demo
* [bootloader-cmd](bootloader-cmd/README.md): Bootloader with command parsing
* [cmd-demo](cmd-demo/README.md): Command parsing demonstration
* [cmd-tx-rtos](cmd-tx-rtos/README.md): Command parsing using FreeRTOS
* [flash-write-demo](flash-write-demo/README.md): Reprograms the Flash
* [flight](flight/README.md): Flight software
* [loopback-rtos](loopback-rtos/README.md): Serial loopback using FreeRTOS
* [uart](uart/README.md): UART demo
* [uart-rtos](uart-rtos/README.md): UART demo using FreeRTOS
* [libopencm3](libopencm3/README.md): Sobmodule library that provides functions
  for Arm Cortex-M MCUs
* [rules.mk](rules.mk): Used by `make`
* [README.md](README.md): This document

## License

Written by Bradley Denby  
Other contributors: None

See the top-level LICENSE file for the license.
