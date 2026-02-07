
#ifndef GRAPHICS_H
#define GRAPHICS_H

#include <stdint.h>
#include <arch/zx.h>

// UDG Indexes
#define UDG_PLAYER  0
#define UDG_WALL    1
#define UDG_EXIT    2
#define UDG_EMPTY   ' '

// Colors
#define COLOR_BG     (PAPER_BLACK | INK_BLACK)
#define COLOR_PLAYER (PAPER_BLACK | INK_WHITE | BRIGHT)
#define COLOR_WALL   (PAPER_BLACK | INK_CYAN)
#define COLOR_EXIT   (PAPER_BLACK | INK_YELLOW | FLASH)

void graphics_init(void);
void graphics_draw_char(uint8_t col, uint8_t row, uint8_t ch, uint8_t attr);
void graphics_cls(uint8_t attr);
void graphics_border(uint8_t color);

#endif
