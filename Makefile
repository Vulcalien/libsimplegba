# Vulcalien's Library Makefile
# version 0.3.5
#
# Adapted for GBA libraries

TARGET := GBA

# === Basic Info ===
OUT_FILENAME := libsimplegba

SRC_DIR := src
OBJ_DIR := obj
BIN_DIR := bin

SRC_SUBDIRS := math memory backup audio debug

# === Compilation ===
CPPFLAGS := -MMD -MP -Iinclude
CFLAGS   := -O3 -fomit-frame-pointer -marm -mcpu=arm7tdmi\
            -Wall -pedantic

ASFLAGS := -mcpu=arm7tdmi -I$(SRC_DIR)

ifeq ($(TARGET),GBA)
    CC := arm-none-eabi-gcc
    AS := arm-none-eabi-as
endif

# === Extensions ===
ifeq ($(TARGET),GBA)
    OBJ_EXT := o
    LIB_EXT := a
endif

# === Commands ===
MKDIR := mkdir -p
RM    := rm -rfv

# === Resources ===

# list of source file extensions
SRC_EXT := c s

# list of source directories
SRC_DIRS := $(SRC_DIR)\
            $(foreach SUBDIR,$(SRC_SUBDIRS),$(SRC_DIR)/$(SUBDIR))

# list of source files
SRC := $(foreach DIR,$(SRC_DIRS),\
         $(foreach EXT,$(SRC_EXT),\
           $(wildcard $(DIR)/*.$(EXT))))

# list of object directories
OBJ_DIRS := $(SRC_DIRS:%=$(OBJ_DIR)/%)

# list of object files
OBJ := $(SRC:%=$(OBJ_DIR)/%.$(OBJ_EXT))

# output file
OUT := $(BIN_DIR)/$(OUT_FILENAME).$(LIB_EXT)

# === Targets ===

.PHONY: all build clean

all: build

build: $(OUT)

clean:
	@$(RM) $(BIN_DIR) $(OBJ_DIR)

# generate library file
$(OUT): $(OBJ) | $(BIN_DIR)
	$(AR) rcs $@ $^

# compile .c files
$(OBJ_DIR)/%.c.$(OBJ_EXT): %.c | $(OBJ_DIRS)
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

# compile .s files
$(OBJ_DIR)/%.s.$(OBJ_EXT): %.s | $(OBJ_DIRS)
	$(AS) $(ASFLAGS) $< -o $@

# create directories
$(BIN_DIR) $(OBJ_DIRS):
	$(MKDIR) "$@"

-include $(OBJ:.$(OBJ_EXT)=.d)
