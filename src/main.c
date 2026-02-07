
#include "game.h"
#include "graphics.h"
#include <arch/zx.h>

int main(void) {
  // Setup
  zx_border(INK_BLACK);
  zx_cls(PAPER_BLACK | INK_WHITE);

  // Init modules
  graphics_init();
  game_init();

  // Run
  game_run();

  return 0;
}
