# Vulcalien's Manual Makefile
# version 0.1.0

# ==================================================================== #
#                              Basic Info                              #
# ==================================================================== #

SRC_DIR := .
OUT_DIR := out

SRC_SUBDIRS := misc hardware cartridge debug

# ==================================================================== #
#                               Commands                               #
# ==================================================================== #

MKDIR := mkdir -p
RM    := rm -rfv

# ==================================================================== #
#                              Resources                               #
# ==================================================================== #

SRC_DIRS := $(SRC_DIR) $(foreach SUB,$(SRC_SUBDIRS),$(SRC_DIR)/$(SUB))

SRC := $(foreach DIR,$(SRC_DIRS),\
         $(wildcard $(DIR)/*.adoc))

# ==================================================================== #
#                               Targets                                #
# ==================================================================== #

.PHONY: all build clean

all: build

build: | $(OUT_DIR)
	asciidoctor -D $(OUT_DIR) -b manpage $(SRC)

clean:
	@$(RM) $(OUT_DIR)

# create directories
$(OUT_DIR):
	$(MKDIR) $@
