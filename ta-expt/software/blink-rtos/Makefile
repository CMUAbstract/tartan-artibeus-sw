PROJECT = blink-rtos
BUILD_DIR = build

SHARED_DIR = ./rtos
CFILES = blink-rtos.c
CFILES += heap_4.c list.c opencm3.c port.c tasks.c
AFILES += # SHARED_DIR ASM files

# Edit these two lines as needed
DEVICE=stm32l496rgt3
OOCD_FILE = board/stm32l4-generic.cfg

# All lines below probably should not be edited
VPATH += $(SHARED_DIR)
INCLUDES += $(patsubst %,-I%, . $(SHARED_DIR))
OPENCM3_DIR=../libopencm3

include $(OPENCM3_DIR)/mk/genlink-config.mk
include ../rules.mk
include $(OPENCM3_DIR)/mk/genlink-rules.mk
