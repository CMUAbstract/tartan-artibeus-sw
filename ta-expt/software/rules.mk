# rules.mk
# Makefile rules
#
# This version of rules.mk expects the following to be defined before inclusion:
# Required:
#   OPENCM3_DIR: libopencm3 directory
#   PROJECT: the basename of the output elf
#   CFILES: list the file names only; e.g. main.c foo.c etc.
#   DEVICE: the full device name, e.g. stm32l496rgt3
#    or
#   LDSCRIPT: the full path, e.g. ../path/to/stm32l496rgt3.ld
#   OPENCM3_LIB: the basename, e.g. opencm3_stm32l4
#   OPENCM3_DEFS: the target define, e.g. -DSTM32L4
#   ARCH_FLAGS: e.g. -mthumb -mcpu=cortex-m4 -mfloat-abi=hard -mfpu=fpv4-sp-d16
#    (i.e. the full set of cpu arch flags; none are defined in this file)
# Optional:
#   INCLUDES: fully formed -I paths (if extra are needed), e.g. -I../shared
#   BUILD_DIR: defaults to bin; set this if building multiarch
#   OPT: full -O flag; defaults to -Os
#   CSTD: defaults -std=c99
#   CXXSTD: no default
#   OOCD_INTERFACE: e.g. stlink-v2
#   OOCD_TARGET: e.g. stm32l4x
#    NOTE: both are used only for the "make flash" target
#   OOCD_FILE: e.g. my.openocd.cfg
#    NOTE: overrides interface/target above and is used as just -f FILE
# Other notes:
#   No support for stylecheck
#   No support for BMP/texane/random flash methods
#   No support for magically finding the library
#   C++ hasn't been actually tested with this
#   Second expansion/secondary not set, add this if you need them

BUILD_DIR ?= build
OPT ?= -Os
CSTD ?= -std=c99

# Silent by default; 'make V=1' shows all compiler calls;
# V=99 shows all sorts of things
V ?= 0
ifeq ($(V),0)
Q := @
NULL := 2>/dev/null
endif

# Tool paths
PREFIX ?= arm-none-eabi-
CC = $(PREFIX)gcc
LD = $(PREFIX)gcc
OBJCOPY = $(PREFIX)objcopy
OBJDUMP = $(PREFIX)objdump
OOCD ?= openocd

OPENCM3_INC = $(OPENCM3_DIR)/include

# Inclusion of library header files
INCLUDES += $(patsubst %,-I%, . $(OPENCM3_INC) )

OBJS = $(CFILES:%.c=$(BUILD_DIR)/%.o)
OBJS += $(AFILES:%.S=$(BUILD_DIR)/%.o)
GENERATED_BINS = $(PROJECT).elf $(PROJECT).bin $(PROJECT).hex $(PROJECT).map $(PROJECT).list $(PROJECT).lss

TGT_CPPFLAGS += -MD
TGT_CPPFLAGS += -Wall -Wundef $(INCLUDES)
TGT_CPPFLAGS += $(INCLUDES) $(OPENCM3_DEFS)

TGT_CFLAGS += $(OPT) $(CSTD) -ggdb3
TGT_CFLAGS += $(ARCH_FLAGS)
TGT_CFLAGS += -fno-common
TGT_CFLAGS += -ffunction-sections -fdata-sections
TGT_CFLAGS += -Wextra -Wshadow -Wno-unused-variable -Wimplicit-function-declaration
TGT_CFLAGS += -Wredundant-decls -Wstrict-prototypes -Wmissing-prototypes

TGT_CXXFLAGS += $(OPT) $(CXXSTD) -ggdb3
TGT_CXXFLAGS += $(ARCH_FLAGS)
TGT_CXXFLAGS += -fno-common
TGT_CXXFLAGS += -ffunction-sections -fdata-sections
TGT_CXXFLAGS += -Wextra -Wshadow -Wredundant-decls  -Weffc++

TGT_ASFLAGS += $(OPT) $(ARCH_FLAGS) -ggdb3

TGT_LDFLAGS += -T$(LDSCRIPT) -L$(OPENCM3_DIR)/lib -nostartfiles
TGT_LDFLAGS += $(ARCH_FLAGS)
TGT_LDFLAGS += -specs=nano.specs
TGT_LDFLAGS += -Wl,--gc-sections
# OPTIONAL
#TGT_LDFLAGS += -Wl,-Map=$(PROJECT).map
ifeq ($(V),99)
TGT_LDFLAGS += -Wl,--print-gc-sections
endif

# Linker script generator fills in this part
ifeq (,$(DEVICE))
LDLIBS += -l$(OPENCM3_LIB)
endif
# nosys is only in newer gcc-arm-embedded
#LDLIBS += -specs=nosys.specs
LDLIBS += -Wl,--start-group -lc -lgcc -lnosys -Wl,--end-group

# Burn in legacy hell fortran modula pascal yacc etc
.SUFFIXES:
.SUFFIXES: .c .S .h .o .cxx .elf .bin .hex .list .lss

# Make should never try to get a file out of source control
%: %,v
%: RCS/%,v
%: RCS/%
%: s.%
%: SCCS/s.%

all: $(PROJECT).elf $(PROJECT).bin $(PROJECT).hex
flash: $(PROJECT).flash

# error if not using linker script generator
ifeq (,$(DEVICE))
$(LDSCRIPT):
ifeq (,$(wildcard $(LDSCRIPT)))
    $(error Unable to find specified linker script: $(LDSCRIPT))
endif
else
# if linker script generator was used, make sure it's cleaned
GENERATED_BINS += $(LDSCRIPT)
endif

# Need a special rule to have a bin dir
$(BUILD_DIR)/%.o: %.c
	@printf "  CC\t$<\n"
	@mkdir -p $(dir $@)
	$(Q)$(CC) $(TGT_CFLAGS) $(CFLAGS) $(TGT_CPPFLAGS) $(CPPFLAGS) -o $@ -c $<

$(BUILD_DIR)/%.o: %.cxx
	@printf "  CXX\t$<\n"
	@mkdir -p $(dir $@)
	$(Q)$(CC) $(TGT_CXXFLAGS) $(CXXFLAGS) $(TGT_CPPFLAGS) $(CPPFLAGS) -o $@ -c $<

$(BUILD_DIR)/%.o: %.S
	@printf "  AS\t$<\n"
	@mkdir -p $(dir $@)
	$(Q)$(CC) $(TGT_ASFLAGS) $(ASFLAGS) $(TGT_CPPFLAGS) $(CPPFLAGS) -o $@ -c $<

$(PROJECT).elf: $(OBJS) $(LDSCRIPT) $(LIBDEPS)
	@printf "  LD\t$@\n"
	$(Q)$(LD) $(TGT_LDFLAGS) $(LDFLAGS) $(OBJS) $(LDLIBS) -o $@

%.bin: %.elf
	@printf "  OBJCOPY\t$@\n"
	$(Q)$(OBJCOPY) -O binary  $< $@

%.hex: %.elf
	@printf "  OBJCOPY\t$@\n"
	$(Q)$(OBJCOPY) -O ihex  $< $@

%.lss: %.elf
	$(OBJDUMP) -h -S $< > $@

%.list: %.elf
	$(OBJDUMP) -S $< > $@

%.flash: %.elf
	@printf "  FLASH\t$<\n"
ifeq (,$(OOCD_FILE))
	$(Q)(echo "halt; program $(realpath $(*).elf) verify reset" | nc -4 localhost 4444 2>/dev/null) || \
		$(OOCD) -f interface/$(OOCD_INTERFACE).cfg \
		-f target/$(OOCD_TARGET).cfg \
		-c "program $(realpath $(*).elf) verify reset exit" \
		$(NULL)
else
	$(Q)(echo "halt; program $(realpath $(*).elf) verify reset" | nc -4 localhost 4444 2>/dev/null) || \
		$(OOCD) -f $(OOCD_FILE) \
		-c "program $(realpath $(*).elf) verify reset exit" \
		$(NULL)
endif

clean:
	rm -rf $(BUILD_DIR) $(GENERATED_BINS)

.PHONY: all clean flash
-include $(OBJS:.o=.d)

