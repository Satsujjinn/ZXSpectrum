
#include "audio.h"
#include <arch/zx.h>

// Simple beep routine using bit-banging directly or relying on bit_beep
// intrinsic usually available. If not, we can assume standard ROM call via
// assembly or simpler beep logic.
//
// Error indicated zx_beep takes fewer params or implicit.
// Actually standard z88dk zx_beep(duration, period) is void.
// However, the error "too many parameters" suggests the compiler sees it
// differently. Let's use `bit_beep(int duration, int period)` which is usually
// safe. Or just declare specific prototype if we know it.

// Let's try bit_beep directly.
extern void bit_beep(int duration, int period);

void audio_play_move(void) {
  // Short high blip
  // Period small = high pitch
  bit_beep(50, 200);
}

void audio_play_hit(void) {
  // Low thud
  bit_beep(100, 2000); // 2000 period = lower freq
}

void audio_play_level_complete(void) {
  unsigned int i;
  for (i = 2000; i > 500; i -= 200) {
    bit_beep(20, i);
  }
}

void audio_play_win(void) {
  unsigned int i;
  for (i = 0; i < 3; i++) {
    audio_play_level_complete();
  }
}
