#include <stdint.h>
#include <stdio.h>
struct rx5voice {
  char name[7];
  uint8_t octave, note, pcmformat, loop;
  int32_t pcmstart, pcmend, loopstart, loopend;
  uint8_t ar, d1r, rr, d2r, d1l, gt;
  uint8_t bendrate, bendrange, unknown, level, channel;
};
void loadvoice(struct rx5voice *v, uint8_t *p);
struct rx5rom {
  struct rx5voice voice[256];
  int nvoice;
  uint8_t data[128 * 1024];
};
void loadrom(struct rx5rom *rom, FILE *f);
