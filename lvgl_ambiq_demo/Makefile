#
# usage:
# 1 cmkae build：make cmake-build BOARD=apollo510_evb DEMO_NAME=lvgl_test
# 2
#
BOARD ?= apollo510_evb
DEMO_NAME ?= lvgl_test

BUILD_DIR = $(CURDIR)/build

PHONY: cmake-build

cmake-build:
	@echo "========================================="
	@echo "Building with configuration:"
	@echo "  BOARD:     $(BOARD)"
	@echo "  DEMO_NAME: $(DEMO_NAME)"
	@echo "  BUILD_DIR: $(BUILD_DIR)"
	@echo "========================================="
	@mkdir -p $(BUILD_DIR)
	@cd $(BUILD_DIR) && \
	cmake .. -G "Unix Makefiles" \
	$(if $(BOARD),-DBOARD=$(BOARD)) \
	$(if $(DEMO_NAME),-DDEMO_NAME=$(DEMO_NAME)) && \
	cmake --build .

clean:
	rm -rf $(BUILD_DIR)

realclean:
	rm -rf $(BUILD_DIR)
