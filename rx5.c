/* rx5.c: library functions for working with Yamaha RX5 rom dumps */
#include "rx5.h"
#include <err.h>
#include <string.h>
int32_t int24(uint8_t *p) {
  return (((int32_t)p[0] & 1) << 16) | ((int32_t)p[1] << 8) | (int32_t)p[2];
}
void loadvoice(struct rx5voice *v, uint8_t *p) {
  memset(v, 0, sizeof(*v));
  v->octave = p[0];
  v->note = p[1];
  v->pcmformat = p[2] & 1;
  v->loop = !(p[3] & 0x40);
  v->pcmstart = int24(p + 3) & ~0xff;
  v->loopstart = int24(p + 5);
  v->loopend = int24(p + 8);
  v->pcmend = int24(p + 11);
  v->ar = p[14];
  v->d1r = p[15];
  v->rr = p[16];
  v->d2r = p[17];
  v->d1l = p[18];
  v->gt = p[19];
  v->bendrate = p[20];
  v->bendrange = p[21];
  v->unknown = p[22];
  v->level = p[23];
  v->channel = p[24];
  memmove(v->name, p + 26, 6);
  v->name[6] = 0;
}
void loadrom(struct rx5rom *rom, FILE *f) {
  int i;
  if (fread(rom->data, 1, sizeof(rom->data), f) != sizeof(rom->data))
    errx(-1, "ROM too small");
  if (fgetc(f) >= 0)
    errx(-1, "ROM too big");
  if (memcmp(rom->data, "\x00\x00\x00\x00", 4))
    errx(-1, "missing leading zeroes");
  rom->nvoice = rom->data[RX5_NUM_VOICE];
  for (i = 0; i < rom->nvoice; i++)
    loadvoice(rom->voice + i, rom->data + 6 + i * 32);
}
