PROJECT = cmd-demo
BUILD_DIR = build

RTOS_DIR = ./rtos
TA_EXPT_DIR = ./ta-expt
CFILES = cmd-demo.c
CFILES += heap_4.c list.c opencm3.c port.c queue.c tasks.c commands.c
AFILES += # TA_EXPT_DIR ASM files

# Edit these two lines as needed
DEVICE=stm32l496rgt3
OOCD_FILE = board/stm32l4-generic.cfg

# All lines below probably should not be edited
VPATH += $(RTOS_DIR)
VPATH += $(TA_EXPT_DIR)
INCLUDES += $(patsubst %,-I%, . $(RTOS_DIR))
INCLUDES += $(patsubst %,-I%, . $(TA_EXPT_DIR))
OPENCM3_DIR=../libopencm3

include $(OPENCM3_DIR)/mk/genlink-config.mk
include ../rules.mk
include $(OPENCM3_DIR)/mk/genlink-rules.mk
