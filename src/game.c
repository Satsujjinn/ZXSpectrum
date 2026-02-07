
#include "game.h"
#include "audio.h"
#include "graphics.h"
#include "levels.h"
#include <arch/zx.h>
#include <compress/zx7.h> // Not used but harmless
#include <input.h>
#include <stdlib.h>

// Declare bit_beep if used directly
extern void bit_beep(int duration, int period);

// Constants
#define MOVE_DELAY 2 // Speed of sliding
#define MAX_UNDO 50  // Max moves to remember

// Global state
static uint8_t current_level_idx = 0;
static uint8_t px, py;
static const Level *current_level;

// Undo Stack
typedef struct {
  uint8_t x;
  uint8_t y;
} Position;

static Position undo_stack[MAX_UNDO];
static int undo_ptr = 0; // Points to NEXT free slot

// Forward declarations
static void load_level(uint8_t idx);
static void play_level(void);

// Logic
static void push_undo(uint8_t x, uint8_t y) {
  if (undo_ptr < MAX_UNDO) {
    undo_stack[undo_ptr].x = x;
    undo_stack[undo_ptr].y = y;
    undo_ptr++;
  } else {
    // Shift stack down
    int i;
    for (i = 0; i < MAX_UNDO - 1; i++) {
      undo_stack[i] = undo_stack[i + 1];
    }
    undo_stack[MAX_UNDO - 1].x = x;
    undo_stack[MAX_UNDO - 1].y = y;
  }
}

static int pop_undo(uint8_t *x, uint8_t *y) {
  if (undo_ptr > 0) {
    undo_ptr--;
    *x = undo_stack[undo_ptr].x;
    *y = undo_stack[undo_ptr].y;
    return 1;
  }
  return 0;
}

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

  // Save state before move starts
  push_undo(px, py);

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

static void undo_move_action(void) {
  uint8_t old_x, old_y;
  // Current pos
  uint8_t cx = px;
  uint8_t cy = py;

  if (pop_undo(&old_x, &old_y)) {
    px = old_x;
    py = old_y;

    draw_tile(cx, cy);
    draw_tile(px, py);

    // Sound: Sweep down
    bit_beep(10, 1000);
  } else {
    // Error sound
    bit_beep(40, 300);
  }
}

static void load_level(uint8_t idx) {
  current_level = levels_get(idx);
  px = current_level->start_x;
  py = current_level->start_y;
  undo_ptr = 0; // Clear undo
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
  const char *inst5 = "U to UNDO";

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
    graphics_draw_char(start_col + i, 18, inst4[i], PAPER_BLACK | INK_YELLOW);

  len = 9;
  start_col = (LEVEL_COLS - len) / 2;
  for (i = 0; i < len; i++)
    graphics_draw_char(start_col + i, 19, inst5[i], PAPER_BLACK | INK_WHITE);

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
    if (ry != 8 && ry != 11 && ry != 13 && ry != 15 && ry != 18 && ry != 19 &&
        ry != 21) {
      graphics_draw_char(rx, ry, '.', PAPER_BLACK | INK_YELLOW);
      bit_beep(2, 4000);
      in_pause(5);
      graphics_draw_char(rx, ry, ' ', PAPER_BLACK | INK_BLACK);
    }
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

    if (in_key_pressed(IN_KEY_SCANCODE_q))
      dy = -1;
    else if (in_key_pressed(IN_KEY_SCANCODE_a))
      dy = 1;
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

    else if (in_key_pressed(IN_KEY_SCANCODE_u)) {
      undo_move_action();
      in_pause(20);
      while (in_key_pressed(IN_KEY_SCANCODE_u)) {
      }
      continue;
    }

    if (dx != 0 || dy != 0) {
      if (slide_player(dx, dy)) {
        if (get_tile_at(px, py) == TILE_EXIT) {
          audio_play_level_complete();
          in_pause(50);
          return;
        }
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

  game_init();
  game_run();
}
