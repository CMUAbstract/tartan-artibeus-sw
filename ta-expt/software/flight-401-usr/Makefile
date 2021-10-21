PROJECT = flight-401-usr
BUILD_DIR = build

TA_EXPT_DIR = ./ta-expt
CFILES = flight_401_usr.c
CFILES += application.c taolst_protocol.c

# Edit these two lines as needed
DEVICE=stm32l496rgt3
OOCD_FILE = board/stm32l4-generic.cfg
LDLIBS += -lm -u _printf_float

# All lines below probably should not be edited
VPATH += $(TA_EXPT_DIR)
INCLUDES += $(patsubst %,-I%, . $(TA_EXPT_DIR))
OPENCM3_DIR=../libopencm3

include $(OPENCM3_DIR)/mk/genlink-config.mk
include ../rules.mk
include $(OPENCM3_DIR)/mk/genlink-rules.mk
