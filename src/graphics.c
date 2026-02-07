
#include "graphics.h"
#include <arch/zx.h>
#include <string.h>

// Combined UDG data (A, B, C...)
// We need 21 characters * 8 bytes = 168 bytes to be safe,
// or just enough for what we use if we don't use D-U.
// System variable 23675 points to start of 'A'.
static const uint8_t game_udgs[] = {
    // A: Player (Radiating Star)
    0x00, 0x18, 0x3C, 0x7E, 0x7E, 0x3C, 0x18, 0x00,
    // B: Wall (Circuit/Brick)
    0xFF, 0x81, 0xBD, 0xA5, 0xA5, 0xBD, 0x81, 0xFF,
    // C: Exit (Portal)
    0x3C, 0x42, 0x99, 0xA5, 0xA5, 0x99, 0x42, 0x3C,
    // Filler for others if needed, but we only use A, B, C (mapped to 0, 1, 2
    // in our defines?)
    // Wait, UDG 'A' is typically char code 144.
    // Our defines in graphics.h were:
    // #define UDG_PLAYER  0
    // #define UDG_WALL    1
    // #define UDG_EXIT    2
    // If we pass 0, 1, 2 to standard printf or zx_ functions, they might print
    // control codes.
    // 0, 1, 2 are not standard printable chars.
    // We should use 'A' (144), 'B' (145), 'C' (146) if we rely on UDG system.
    // OR we can just pass the pointer to the 8-byte data to a custom draw
    // routine.

    // In graphics_draw_char, we use:
    // *zx_cxy2saddr(row, col) = ch;
    // This puts the byte `ch` into video memory.
    // If we put 144, the ULA/renderer logic will look up the font.
    // The Spectrum ROM font lookup:
    // 0-31: Control ??
    // 32-127: Standard ASCII (ROM)
    // 128-143: Block graphics (Calculated?)
    // 144-164: UDG (Look at 23675)

    // So if we start UDG at 144 ('\220'), we should use 144, 145, 146.
};

// Redefine 0,1,2 to actual UDG codes
#undef UDG_PLAYER
#undef UDG_WALL
#undef UDG_EXIT
// Standard z88dk / Spectrum: 'A' is \x90 (144)
#define CHAR_UDG_A '\x90'
#define CHAR_UDG_B '\x91'
#define CHAR_UDG_C '\x92'

void graphics_init(void) {
  // Point UDG system variable (23675) to our data
  // 23675 is a standard location in 48K ROM/sysvars
  *(uint16_t *)23675 = (uint16_t)game_udgs;
}

void graphics_draw_char(uint8_t col, uint8_t row, uint8_t ch, uint8_t attr) {
  if (col >= 32 || row >= 24)
    return;

  // Map internal IDs to UDG chars if needed
  // In game.c we use constants UDG_PLAYER=0 etc.
  // We need to map them here or change game.c/graphics.h
  // Let's map simpler here:
  uint8_t final_ch = ch;
  if (ch == 0)
    final_ch = CHAR_UDG_A; // Player
  else if (ch == 1)
    final_ch = CHAR_UDG_B; // Wall
  else if (ch == 2)
    final_ch = CHAR_UDG_C; // Exit

  *zx_cxy2saddr(row, col) = final_ch;
  *zx_cxy2aaddr(row, col) = attr;
}

void graphics_cls(uint8_t attr) { zx_cls(attr); }

void graphics_border(uint8_t color) { zx_border(color); }
