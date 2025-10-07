#include <stdint.h>
#include <stdio.h>
struct rx5voice {
  char name[7];
  uint8_t octave, note, pcmformat, loop;
  int32_t pcmstart, pcmend, loopstart, loopend;
  uint8_t ar, d1r, rr, d2r, d1l, gt;
  uint8_t bendrate, bendrange, unknown, level, channel;
};
#define RX5_ROM_SIZE (128 * 1024)
struct rx5rom {
  struct rx5voice voice[256];
  int nvoice;
  uint8_t data[RX5_ROM_SIZE];
};
void loadrom(struct rx5rom *rom, FILE *f);
void storevoices(struct rx5rom *rom);
