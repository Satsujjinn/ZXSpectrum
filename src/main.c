/*
 * LUMEN — a minimal art game for the Sinclair ZX Spectrum 48K
 * You are light. Move through three chambers. Reach the star.
 *
 * Controls: 5 left, 6 right, 8 up, 7 down (Spectrum keys)
 *           Q left, P right, O up, A down
 *
 * Build: make   Run: make run   Debug: F5 (DeZog)
 */

#include <arch/zx.h>
#include <input.h>
#include <stdint.h>

#define COLS  32
#define ROWS  22
#define TILE_EMPTY  0
#define TILE_WALL   1
#define TILE_EXIT   2

#define CHR_EMPTY   ' '
#define CHR_WALL    255
#define CHR_EXIT    '*'
#define CHR_PLAYER  'O'

static uint8_t level[ROWS][COLS];
static uint8_t px, py;
static uint8_t level_index;
static uint8_t level_attr_bg;
static uint8_t level_attr_wall;
static uint8_t level_attr_exit;
static uint8_t border_colour;

/* Level 1: Blue chamber — frame and exit top-right */
static const uint8_t level1[ROWS][COLS] = {
    {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,2},
    {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
    {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
    {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
    {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
    {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
    {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
    {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
    {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
    {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
    {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
    {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
    {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
    {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
    {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
    {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
    {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
    {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
    {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
    {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
    {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
    {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
};

/* Level 2: Dark chamber — diagonal and exit bottom-left */
static const uint8_t level2[ROWS][COLS] = {
    {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
    {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
    {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
    {1,0,0,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
    {1,0,0,0,0,0,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
    {1,0,0,0,0,0,0,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
    {1,0,0,0,0,0,0,0,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
    {1,0,0,0,0,0,0,0,0,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
    {1,0,0,0,0,0,0,0,0,0,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
    {1,0,0,0,0,0,0,0,0,0,0,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
    {1,0,0,0,0,0,0,0,0,0,0,0,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
    {1,0,0,0,0,0,0,0,0,0,0,0,0,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
    {1,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
    {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
    {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
    {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,0,0,0,0,0,0,0,0,0,0,0,0,1},
    {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,0,0,0,0,0,0,0,0,0,0,0,1},
    {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,0,0,0,0,0,0,0,0,0,0,1},
    {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,0,0,0,0,0,0,0,0,0,1},
    {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,0,0,0,0,0,0,0,0,1},
    {1,2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,0,0,0,0,0,0,0,1},
    {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
};

/* Level 3: Final — central pillar, exit right */
static const uint8_t level3[ROWS][COLS] = {
    {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
    {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
    {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
    {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
    {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
    {1,0,0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
    {1,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
    {1,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
    {1,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
    {1,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
    {1,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
    {1,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
    {1,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
    {1,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
    {1,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,2,0,0,0,1},
    {1,0,0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
    {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
    {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
    {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
    {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
    {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
    {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
};

/* Copy level data and set player start (away from exit) */
static void load_level(uint8_t idx)
{
    const uint8_t (*src)[COLS];
    uint8_t r, c;

    switch (idx) {
        case 0: src = level1; break;
        case 1: src = level2; break;
        default: src = level3; break;
    }

    for (r = 0; r < ROWS; r++)
        for (c = 0; c < COLS; c++)
            level[r][c] = src[r][c];

    /* Start positions per level */
    if (idx == 0)      { px = 1;  py = 1;  }
    else if (idx == 1) { px = 30; py = 1;  }
    else               { px = 1;  py = 1;  }

    /* Level palettes — each chamber has its own mood */
    if (idx == 0) {
        level_attr_bg   = PAPER_BLUE | INK_BLUE;
        level_attr_wall = PAPER_BLUE | INK_CYAN;
        level_attr_exit = PAPER_BLUE | INK_YELLOW | BRIGHT;
        border_colour   = INK_CYAN;
    } else if (idx == 1) {
        level_attr_bg   = PAPER_BLACK | INK_BLACK;
        level_attr_wall = PAPER_BLACK | INK_MAGENTA;
        level_attr_exit = PAPER_BLACK | INK_GREEN | BRIGHT;
        border_colour   = INK_MAGENTA;
    } else {
        level_attr_bg   = PAPER_RED | INK_RED;
        level_attr_wall = PAPER_RED | INK_YELLOW;
        level_attr_exit = PAPER_RED | INK_WHITE | BRIGHT;
        border_colour   = INK_YELLOW;
    }
}

/* Spectrum: zx_cxy2saddr(row, col) — row first, then column */
static void draw_cell(uint8_t col, uint8_t row, uint8_t ch, uint8_t attr)
{
    *zx_cxy2saddr(row, col) = ch;
    *zx_cxy2aaddr(row, col) = attr;
}

static void draw_level(void)
{
    uint8_t r, c;
    uint8_t ch, attr;

    zx_cls(level_attr_bg);
    zx_border(border_colour);

    for (r = 0; r < ROWS; r++)
        for (c = 0; c < COLS; c++) {
            switch (level[r][c]) {
                case TILE_WALL: ch = CHR_WALL; attr = level_attr_wall; break;
                case TILE_EXIT: ch = CHR_EXIT; attr = level_attr_exit; break;
                default:        ch = CHR_EMPTY; attr = level_attr_bg; break;
            }
            draw_cell(c, r, ch, attr);
        }
}

static void draw_player(void)
{
    draw_cell(px, py, CHR_PLAYER, level_attr_bg | INK_WHITE | BRIGHT);
}

static uint8_t tile_at(uint8_t c, uint8_t r)
{
    if (c >= COLS || r >= ROWS) return TILE_WALL;
    return level[r][c];
}

static void game_loop(void)
{
    uint8_t nx, ny;

    for (;;) {
        draw_level();
        draw_player();

        /* Small delay for playable speed */
        in_pause(120);

        if (in_key_pressed(IN_KEY_SCANCODE_5) || in_key_pressed(IN_KEY_SCANCODE_q)) {
            nx = px; if (px > 0) nx = px - 1;
            if (tile_at(nx, py) != TILE_WALL) px = nx;
        }
        if (in_key_pressed(IN_KEY_SCANCODE_6) || in_key_pressed(IN_KEY_SCANCODE_p)) {
            nx = px; if (px < COLS - 1) nx = px + 1;
            if (tile_at(nx, py) != TILE_WALL) px = nx;
        }
        if (in_key_pressed(IN_KEY_SCANCODE_8) || in_key_pressed(IN_KEY_SCANCODE_o)) {
            ny = py; if (py > 0) ny = py - 1;
            if (tile_at(px, ny) != TILE_WALL) py = ny;
        }
        if (in_key_pressed(IN_KEY_SCANCODE_7) || in_key_pressed(IN_KEY_SCANCODE_a)) {
            ny = py; if (py < ROWS - 1) ny = py + 1;
            if (tile_at(px, ny) != TILE_WALL) py = ny;
        }

        if (tile_at(px, py) == TILE_EXIT) {
            level_index++;
            if (level_index >= 3) return; /* finished */
            load_level(level_index);
        }
    }
}

static void title_screen(void)
{
    zx_cls(PAPER_BLACK | INK_BLACK);
    zx_border(INK_CYAN);

    /* LUMEN — centered, bold */
    draw_cell(13, 9,  'L', PAPER_BLACK | INK_WHITE | BRIGHT);
    draw_cell(14, 9,  'U', PAPER_BLACK | INK_WHITE | BRIGHT);
    draw_cell(15, 9,  'M', PAPER_BLACK | INK_WHITE | BRIGHT);
    draw_cell(16, 9,  'E', PAPER_BLACK | INK_WHITE | BRIGHT);
    draw_cell(17, 9,  'N', PAPER_BLACK | INK_WHITE | BRIGHT);

    draw_cell(12, 12, '5', PAPER_BLACK | INK_CYAN);
    draw_cell(13, 12, '6', PAPER_BLACK | INK_CYAN);
    draw_cell(14, 12, '7', PAPER_BLACK | INK_CYAN);
    draw_cell(15, 12, '8', PAPER_BLACK | INK_CYAN);
    draw_cell(16, 12, ' ', PAPER_BLACK | INK_CYAN);
    draw_cell(17, 12, 'M', PAPER_BLACK | INK_CYAN);
    draw_cell(18, 12, 'O', PAPER_BLACK | INK_CYAN);
    draw_cell(19, 12, 'V', PAPER_BLACK | INK_CYAN);
    draw_cell(20, 12, 'E', PAPER_BLACK | INK_CYAN);

    draw_cell(11, 14, 'R', PAPER_BLACK | INK_YELLOW);
    draw_cell(12, 14, 'E', PAPER_BLACK | INK_YELLOW);
    draw_cell(13, 14, 'A', PAPER_BLACK | INK_YELLOW);
    draw_cell(14, 14, 'C', PAPER_BLACK | INK_YELLOW);
    draw_cell(15, 14, 'H', PAPER_BLACK | INK_YELLOW);
    draw_cell(16, 14, ' ', PAPER_BLACK | INK_YELLOW);
    draw_cell(17, 14, '*', PAPER_BLACK | INK_YELLOW | BRIGHT);

    in_wait_key();
}

static void end_screen(void)
{
    zx_cls(PAPER_BLACK | INK_BLACK);
    zx_border(INK_YELLOW);

    draw_cell(14, 10, 'L', PAPER_BLACK | INK_YELLOW | BRIGHT);
    draw_cell(15, 10, 'I', PAPER_BLACK | INK_YELLOW | BRIGHT);
    draw_cell(16, 10, 'G', PAPER_BLACK | INK_YELLOW | BRIGHT);
    draw_cell(17, 10, 'H', PAPER_BLACK | INK_YELLOW | BRIGHT);
    draw_cell(18, 10, 'T', PAPER_BLACK | INK_YELLOW | BRIGHT);

    in_wait_key();
}

int main(void)
{
    title_screen();
    level_index = 0;
    load_level(0);
    game_loop();
    end_screen();
    return 0;
}
