# Tartan Artibeus Experiment Board Flight 401 Application Software

Tartan Artibeus experiment board application flight 401 software

```bash
cd ../../scripts/
source sourcefile.txt
cd ../software/flight-401-usr/
make
st-flash write flight-401-usr.bin 0x8008000
```

**Important note**: In file `generated.stm32l496rgt3.ld`, the line
`rom (rx) : ORIGIN = 0x08000000, LENGTH = 1024K` must be changed to
`rom (rx) : ORIGIN = 0x08008000, LENGTH = 1024K`.

## License

Written by Bradley Denby  
Other contributors: None

See the top-level LICENSE file for the license.
