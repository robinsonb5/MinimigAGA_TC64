# MINIMIG-DE1
# ctrl boot firmware
# 2012, rok.krajnc@gmail.com
# Ported to EightThirtyTwo by AMR

### tools ###

BASE	= 
832DIR = ../../EightThirtyTwo
CC      = $(832DIR)/vbcc/bin/vbcc832
GCC		= gcc
LD      = $(832DIR)/832a/832l
AS      = $(832DIR)/832a/832a
CP      = objcopy
DUMP    = objdump

LIBDIR = $(832DIR)/lib832
BUILD_DIR = 832obj
COMMON_DIR = $(832DIR)/Lib
INCLUDE_DIR = $(832DIR)/include
STARTUP_DIR = $(832DIR)/Lib

PROJECT=832OSDAD

### flags ###

CFLAGS  = -+ -c99 -O=1343 -unsigned-char -D__geta4= -DNULL=0 -I. -I$(INCLUDE_DIR)/ -I$(COMMON_DIR)/ -I../OSD/
ASFLAGS = -eb
LDFLAGS  = -eb -s_STACKSIZE=0x0 -s_STACKTOP=0x2000 -b 0x2000

### variables ###

# headers
HEADERS=spr_defs.h or32_defs.h hardware.h string.h swap.h errors.h mmc.h fat.h

### variables ###
BUILD_USER=\"$(USER)\"
BUILD_T=$$(date +%Y-%m-%d)
BUILD_TIME=\"$(BUILD_T)\"
BUILD_N=$$(cat build_num.txt)
BUILD_NUM=\"$(BUILD_N)\"
BUILD_R=$$(git rev-parse --verify HEAD)
BUILD_REV=\"$(BUILD_R)\"
BUILD_TG=$$(git describe --tags --abbrev=0)
BUILD_TAG=\"$(BUILD_TG)\"

BINDIR=bin
OBJDIR=obj

CFLAGS += -D__BUILD_USER=$(BUILD_USER) -D__BUILD_TAG=$(BUILD_TAG)

### fw specific ###



# common sources
COMMON_SOURCES=$(BUILD_DIR)/hardware.o $(BUILD_DIR)/config.o $(BUILD_DIR)/fat.o $(BUILD_DIR)/fdd.o $(BUILD_DIR)/fpga.o \
	$(BUILD_DIR)/boot.o $(BUILD_DIR)/hdd.o $(BUILD_DIR)/main.o $(BUILD_DIR)/menu.o $(BUILD_DIR)/mmc.o $(BUILD_DIR)/spi.o \
	$(BUILD_DIR)/osd.o $(BUILD_DIR)/rafile.o $(BUILD_DIR)/swap.o $(BUILD_DIR)/hexdump.o $(BUILD_DIR)/uart.o $(BUILD_DIR)/errors.o \
	$(BUILD_DIR)/audiotrack.o $(BUILD_DIR)/bincue.o $(BUILD_DIR)/interrupts.o $(BUILD_DIR)/c64keys.o $(BUILD_DIR)/akiko.o \
	$(BUILD_DIR)/audio.o $(BUILD_DIR)/drivesounds.o


# all sources
ALL_SOURCES = $(LIBDIR)/crt0.a $(COMMON_SOURCES) $(LIBDIR)/lib832.a 


### build rules ###

# all
all: bootcustominit.h version.h $(PROJECT).bin

force:

version.h: force
	echo >version.h \#define MM_FIRMWARE_VERSION \"`date +%Y-%-m-%-d`\"
	touch fpga.c

bootcustominit.h: bootcustominit.s
	vasmm68k_mot -Fbin bootcustominit.s -o bootcustominit.bin
	xxd -i bootcustominit.bin > bootcustominit.h

$(PROJECT).bin: Makefile $(ALL_SOURCES)
	$(LD) $(LDFLAGS) -M $(PROJECT).map -o $@ $(ALL_SOURCES)

# clean
clean:
	@echo clean
	@rm -rf $(BUILD_DIR)/*.o
	@rm -rf $(PROJECT).bin
	@rm -rf $(PROJECT).map

%.asm: %.c
	$(CC) $(CFLAGS) -o=$@ $<

$(BUILD_DIR)/%.o: %.asm $(BUILD_DIR)
	$(AS) $(ASFLAGS) -o $@ $<

$(BUILD_DIR)/%.o: $(STARTUP_DIR)/%.S $(BUILD_DIR) 
	$(AS) $(ASFLAGS) -o $@ $<

$(BUILD_DIR):
	mkdir $(BUILD_DIR)

