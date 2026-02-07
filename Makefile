# ZX Spectrum (48K) build with z88dk
# Output: game.tap (tape), game.map (for DeZog)

TARGET     = game
TAP        = $(TARGET).tap
MAP        = $(TARGET).map
SOURCES    = src/main.c
ZCC        = zcc
ZCC_FLAGS  = +zx -clib=sdcc_iy -v -m
ZCC_LINK   = -o $(TARGET) -create-app

.PHONY: all clean run

all: $(TAP)

$(TAP): $(SOURCES)
	$(ZCC) $(ZCC_FLAGS) $(SOURCES) $(ZCC_LINK)

clean:
	rm -f $(TARGET) $(TARGET).* *.bin *.tap *.map

# Path to Fuse.app (change if needed)
FUSE_APP ?= /Users/leonjordaan/Downloads/Fuse for macOS 2/Fuse.app

run: $(TAP)
	open -a "$(FUSE_APP)" "$(TAP)" 2>/dev/null || open "$(TAP)" || echo "Open $(TAP) in Fuse (File → Open or drag .tap onto Fuse)"
