#******************************************************************************
# Top-level Makefile for lvgl_ambiq_demo
#******************************************************************************

.PHONY: all clean help

# Available boards and projects
BOARDS := apollo510_evb apollo510b_evb apollo5b_eb_revb
PROJECTS := lvgl_test lvgl_ttf lvgl_smartwatch lvgl_cellular_menu

export TOP_DIR := ..

# Build all
all:
	@for project in $(PROJECTS); do \
		for board in $(BOARDS); do \
			echo "Building $$project for $$board..."; \
			$(MAKE) -C $$project/$$board 2>/dev/null || \
			$(MAKE) PROJECT=$$project BOARD=$$board build_target; \
		done; \
	done

# Build specific project+board
# Usage: make PROJECT=lvgl_test BOARD=apollo510_evb
ifneq ($(PROJECT),)
ifneq ($(BOARD),)
# Include board definitions first
include board.mk
# Include project-specific configuration
include $(PROJECT)/demo_common.mk

# Load board configuration BEFORE including common.mk
# This sets BSP, PART, CPU, etc. variables
$(eval $(board_$(BOARD)))

# Now include common.mk (BSP is now set)
include common.mk

# Set paths for top-level build
CONFIG := build/$(PROJECT)/$(BOARD)
PROJECT_SRC := $(PROJECT)/src
INCLUDES += -I$(PROJECT_SRC)
VPATH += :$(PROJECT_SRC)

.PHONY: build_target
build_target: all
endif
endif

clean:
	rm -rf build/

help:
	@echo "Usage: make [PROJECT=name] [BOARD=name]"
	@echo "Projects: $(PROJECTS)"
	@echo "Boards: $(BOARDS)"
	@echo "Example: make PROJECT=lvgl_test BOARD=apollo510_evb"
