
#include "game.h"
#include "audio.h"
#include "graphics.h"
#include "levels.h"
#include <arch/zx.h>
#include <compress/zx7.h> // Not used but harmless
#include <input.h>
#include <stdlib.h>

// Declare bit_beep if used directly
// Note: z88dk sometimes hides this depending on options
extern void bit_beep(int duration, int period);

// Global state
static uint8_t current_level_idx = 0;
static uint8_t px, py;
static const Level *current_level;

// Constants
#define MOVE_DELAY 2 // Speed of sliding

// Forward declarations
static void load_level(uint8_t idx);
static void play_level(void);

// Logic
static uint8_t get_tile_at(uint8_t x, uint8_t y) {
  if (x >= LEVEL_COLS || y >= LEVEL_ROWS)
    return TILE_WALL;
  return current_level->data[y][x];
}

static void draw_tile(uint8_t x, uint8_t y) {
  uint8_t tile = get_tile_at(x, y);
  uint8_t ch, attr;

  if (x == px && y == py) {
    ch = UDG_PLAYER;
    attr = COLOR_PLAYER;
  } else {
    switch (tile) {
    case TILE_WALL:
      ch = UDG_WALL;
      attr = current_level->color_wall;
      break;
    case TILE_EXIT:
      ch = UDG_EXIT;
      attr = current_level->color_exit;
      break;
    default:
      ch = UDG_EMPTY;
      attr = current_level->color_bg;
      break;
    }
  }
  graphics_draw_char(x, y, ch, attr);
}

static void draw_all(void) {
  uint8_t r, c;
  for (r = 0; r < LEVEL_ROWS; r++) {
    for (c = 0; c < LEVEL_COLS; c++) {
      draw_tile(c, r);
    }
  }
}

static uint8_t can_move_to(uint8_t x, uint8_t y) {
  return get_tile_at(x, y) != TILE_WALL;
}

static uint8_t slide_player(int8_t dx, int8_t dy) {
  uint8_t moved = 0;

  if (!can_move_to(px + dx, py + dy)) {
    audio_play_hit();
    return 0;
  }

  while (can_move_to(px + dx, py + dy)) {
    uint8_t ox = px;
    uint8_t oy = py;
    px += dx;
    py += dy;

    draw_tile(ox, oy);
    draw_tile(px, py);

    audio_play_move();
    if (MOVE_DELAY > 0)
      in_pause(MOVE_DELAY);
    moved = 1;
  }
  audio_play_hit();
  return moved;
}

static void load_level(uint8_t idx) {
  current_level = levels_get(idx);
  px = current_level->start_x;
  py = current_level->start_y;
  graphics_cls(current_level->color_bg);
  graphics_border(current_level->border);
  draw_all();
}

static void game_title_screen(void) {
  graphics_cls(PAPER_BLACK | INK_BLACK);
  graphics_border(INK_BLACK);

  // Title: T H E  G A M E
  const char *title = "T H E  G A M E";
  uint8_t len = 14;
  uint8_t start_col = (LEVEL_COLS - len) / 2;
  uint8_t i;
  for (i = 0; i < len; i++) {
    graphics_draw_char(start_col + i, 8, title[i],
                       PAPER_BLACK | INK_WHITE | BRIGHT);
  }

  // Instructions (centered)
  const char *inst1 = "YOU ARE LIGHT";
  const char *inst2 = "MOVE TO STOP";
  const char *inst3 = "REACH THE SINGULARITY";
  const char *inst4 = "Q A O P  or  5 6 7 8";

  len = 13;
  start_col = (LEVEL_COLS - len) / 2;
  for (i = 0; i < len; i++)
    graphics_draw_char(start_col + i, 11, inst1[i], PAPER_BLACK | INK_CYAN);

  len = 12;
  start_col = (LEVEL_COLS - len) / 2;
  for (i = 0; i < len; i++)
    graphics_draw_char(start_col + i, 13, inst2[i], PAPER_BLACK | INK_CYAN);

  len = 21;
  start_col = (LEVEL_COLS - len) / 2;
  for (i = 0; i < len; i++)
    graphics_draw_char(start_col + i, 15, inst3[i],
                       PAPER_BLACK | INK_MAGENTA | BRIGHT);

  len = 20;
  start_col = (LEVEL_COLS - len) / 2;
  for (i = 0; i < len; i++)
    graphics_draw_char(start_col + i, 19, inst4[i], PAPER_BLACK | INK_YELLOW);

  const char *sub = "PRESS ANY KEY";
  len = 13;
  start_col = (LEVEL_COLS - len) / 2;
  for (i = 0; i < len; i++)
    graphics_draw_char(start_col + i, 21, sub[i],
                       PAPER_BLACK | INK_WHITE | FLASH);

  // Animation loop
  while (!in_key_pressed(IN_KEY_SCANCODE_SPACE) &&
         !in_key_pressed(IN_KEY_SCANCODE_ENTER)) {
    // Sparkle
    uint8_t rx = rand() % LEVEL_COLS;
    uint8_t ry = rand() % LEVEL_ROWS;
    // Avoid overwriting text roughly (rows 8, 11, 13, 15, 19, 21)
    if (ry != 8 && ry != 11 && ry != 13 && ry != 15 && ry != 19 && ry != 21) {
      graphics_draw_char(rx, ry, '.', PAPER_BLACK | INK_YELLOW);
      bit_beep(2, 4000);
      in_pause(5);
      graphics_draw_char(rx, ry, ' ', PAPER_BLACK | INK_BLACK);
    }
    // Check for ANY key really, to start
    if (in_inkey() != 0)
      break;
  }

  graphics_cls(PAPER_WHITE | INK_WHITE);
  in_pause(10);
}

static void play_level(void) {
  while (1) {
    int8_t dx = 0;
    int8_t dy = 0;

    // Scan codes generally use lowercase in z88dk/input.h
    if (in_key_pressed(IN_KEY_SCANCODE_q))
      dy = -1;
    else if (in_key_pressed(IN_KEY_SCANCODE_a))
      dy = 1; // Down was 1
    else if (in_key_pressed(IN_KEY_SCANCODE_o))
      dx = -1;
    else if (in_key_pressed(IN_KEY_SCANCODE_p))
      dx = 1;
    else if (in_key_pressed(IN_KEY_SCANCODE_5))
      dx = -1;
    else if (in_key_pressed(IN_KEY_SCANCODE_6))
      dy = 1;
    else if (in_key_pressed(IN_KEY_SCANCODE_7))
      dy = -1;
    else if (in_key_pressed(IN_KEY_SCANCODE_8))
      dx = 1;

    if (dx != 0 || dy != 0) {
      slide_player(dx, dy);

      if (get_tile_at(px, py) == TILE_EXIT) {
        audio_play_level_complete();
        in_pause(50);
        return;
      }

      in_pause(20);
      while (in_key_pressed(IN_KEY_SCANCODE_q) ||
             in_key_pressed(IN_KEY_SCANCODE_p) ||
             in_key_pressed(IN_KEY_SCANCODE_o) ||
             in_key_pressed(IN_KEY_SCANCODE_a) ||
             in_key_pressed(IN_KEY_SCANCODE_5) ||
             in_key_pressed(IN_KEY_SCANCODE_6) ||
             in_key_pressed(IN_KEY_SCANCODE_7) ||
             in_key_pressed(IN_KEY_SCANCODE_8)) {
      }
    }
  }
}

void game_init(void) {
  current_level_idx = 0;
  srand(0);
}

void game_run(void) {
  game_title_screen();

  while (current_level_idx < levels_count()) {
    load_level(current_level_idx);
    play_level();
    current_level_idx++;
    graphics_cls(PAPER_WHITE | INK_WHITE | BRIGHT);
    in_pause(10);
  }

  graphics_cls(PAPER_BLACK | INK_WHITE | BRIGHT);
  audio_play_win();
  in_pause(500);
}
