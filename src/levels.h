
#ifndef LEVELS_H
#define LEVELS_H

#include <stdint.h>

#define LEVEL_COLS 32
#define LEVEL_ROWS 22

#define TILE_EMPTY 0
#define TILE_WALL 1
#define TILE_EXIT 2

// Level metadata
typedef struct {
  const uint8_t (*data)[LEVEL_COLS];
  uint8_t start_x;
  uint8_t start_y;
  uint8_t color_bg;
  uint8_t color_wall;
  uint8_t color_exit;
  uint8_t border;
} Level;

// Get total number of levels
uint8_t levels_count(void);

// Get level data by index (0-based)
const Level *levels_get(uint8_t index);

#endif
