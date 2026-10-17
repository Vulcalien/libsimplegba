# Vulcalien's Executable Makefile
# version 0.3.7
#
# Adapted for GBA testing

TARGET := GBA

# ==================================================================== #
#                              Basic Info                              #
# ==================================================================== #

OUT_FILENAME := test

SRC_DIR := src
OBJ_DIR := obj
BIN_DIR := bin

SRC_SUBDIRS :=

# ==================================================================== #
#                             Compilation                              #
# ==================================================================== #

CPPFLAGS := -MMD -MP -Iinclude -I../include
CFLAGS := -O3 -fomit-frame-pointer -marm -mcpu=arm7tdmi -Wall -pedantic

ASFLAGS := -mcpu=arm7tdmi

LDFLAGS := -nostartfiles -Wl,--print-memory-usage\
           -L../bin -T../files/gba.ld
LDLIBS  := -lsimplegba

ifeq ($(TARGET),GBA)
    CC := arm-none-eabi-gcc
    AS := arm-none-eabi-as
    OBJCOPY := arm-none-eabi-objcopy

    EMULATOR := mgba-qt
endif

# ==================================================================== #
#                        Extensions & Commands                         #
# ==================================================================== #

ifeq ($(TARGET),GBA)
    OBJ_EXT := o
    ELF_EXT := elf
    GBA_EXT := gba
endif

MKDIR := mkdir -p
RM    := rm -rfv

# ==================================================================== #
#                              Resources                               #
# ==================================================================== #

SRC_EXT := c s

SRC_DIRS := $(SRC_DIR) $(foreach SUB,$(SRC_SUBDIRS),$(SRC_DIR)/$(SUB))
OBJ_DIRS := $(SRC_DIRS:%=$(OBJ_DIR)/%)

SRC := $(foreach DIR,$(SRC_DIRS),\
         $(foreach EXT,$(SRC_EXT),\
           $(wildcard $(DIR)/*.$(EXT))))

OBJ := $(SRC:%=$(OBJ_DIR)/%.$(OBJ_EXT))

OUT_ELF := $(BIN_DIR)/$(OUT_FILENAME).$(ELF_EXT)
OUT     := $(BIN_DIR)/$(OUT_FILENAME).$(GBA_EXT)

# ==================================================================== #
#                               Targets                                #
# ==================================================================== #

.PHONY: all run build clean

all: build

run:
	$(EMULATOR) $(OUT)

build: $(OUT)

clean:
	@$(RM) $(BIN_DIR) $(OBJ_DIR)

# generate GBA file
$(OUT): $(OUT_ELF)
	$(OBJCOPY) -O binary $^ $@

# generate ELF file
$(OUT_ELF): ../bin/libsimplegba.a $(OBJ) | $(BIN_DIR)
	$(CC) $(LDFLAGS) $^ $(LDLIBS) -o $@

# compile .c files
$(OBJ_DIR)/%.c.$(OBJ_EXT): %.c | $(OBJ_DIRS)
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

# compile .s files
$(OBJ_DIR)/%.s.$(OBJ_EXT): %.s | $(OBJ_DIRS)
	$(AS) $(ASFLAGS) $< -o $@

# create directories
$(BIN_DIR) $(OBJ_DIRS):
	$(MKDIR) $@

# build libsimplegba
../bin/libsimplegba.a:
	$(MAKE) -C .. build

-include $(OBJ:.$(OBJ_EXT)=.d)
