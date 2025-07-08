
ifeq ($(OS),Windows_NT)
	JLINK := JLink.exe
else
	UNAME_S := $(shell uname -s)
	ifeq ($(UNAME_S),Linux)
		JLINK := JLinkExe
	else ifeq ($(UNAME_S),Darwin)
		JLINK := JLinkExe
	else
		$(error OS not support)
	endif
endif
# Use the JLink tool to flash the board
# Usage: make apollo510_evb or make apollo5b_eb_revb
.PHONY: apollo510_evb
.PHONY: apollo5b_eb_revb
# Flash the board with the appropriate binary file
# Make sure the binary file is located in the same directory as the JLink script
apollo510_evb:
	$(JLINK) -commanderscript ./apollo510_evb/flash.jlink

apollo5b_eb_revb:
	$(JLINK) -commanderscript ./apollo5b_eb_revb/flash.jlink
