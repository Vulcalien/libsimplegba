# Vulcalien's Library Makefile
# version 0.3.3
#
# Adapted for GBA libraries

# === Detect OS ===
ifeq ($(OS),Windows_NT)
    CURRENT_OS := WINDOWS
else
    CURRENT_OS := UNIX
endif

# === Basic Info ===
OUT_FILENAME := base

SRC_DIR := src
OBJ_DIR := obj
BIN_DIR := bin

SRC_SUBDIRS := sound

# === Compilation ===
CPPFLAGS := -Iinclude -MMD -MP
CFLAGS   := -O3 -fomit-frame-pointer -marm -mcpu=arm7tdmi\
            -Wall -pedantic

ASFLAGS := -mcpu=arm7tdmi -I$(SRC_DIR)

ifeq ($(CURRENT_OS),UNIX)
    CC := arm-none-eabi-gcc
    AS := arm-none-eabi-as
else ifeq ($(CURRENT_OS),WINDOWS)
    CC :=
    AS :=
endif

# === Extensions & Commands ===
OBJ_EXT := o
LIB_EXT := a

ifeq ($(CURRENT_OS),UNIX)
    MKDIR := mkdir -p
    RM    := rm -rfv
else ifeq ($(CURRENT_OS),WINDOWS)
    MKDIR := mkdir
    RM    := rmdir /Q /S
endif

# === Resources ===

# list of source file extensions
SRC_EXT := s c

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

clean::
	@$(RM) $(BIN_DIR) $(OBJ_DIR)

# generate library file
$(OUT): $(OBJ) | $(BIN_DIR)
	$(AR) rcs $@ $^

# compile .s files
$(OBJ_DIR)/%.s.$(OBJ_EXT): %.s | $(OBJ_DIRS)
	$(AS) $(ASFLAGS) -o $@ $<

# compile .c files
$(OBJ_DIR)/%.c.$(OBJ_EXT): %.c | $(OBJ_DIRS)
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

# create directories
$(BIN_DIR) $(OBJ_DIRS):
	$(MKDIR) "$@"

-include $(OBJ:.$(OBJ_EXT)=.d)

-include doc/manpages.mk
