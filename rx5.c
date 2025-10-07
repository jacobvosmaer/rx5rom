/* rx5.c: library functions for working with Yamaha RX5 rom dumps */
#include "rx5.h"
#include <err.h>
#include <string.h>
int32_t getaddr(uint8_t *p) {
  return (((int32_t)p[0] & 1) << 16) | ((int32_t)p[1] << 8) | (int32_t)p[2];
}
void putaddr(uint8_t *p, int32_t x) {
  p[0] = (x >> 16) & 1;
  p[1] = x >> 8;
  p[2] = x;
}
void loadvoice(struct rx5voice *v, uint8_t *p) {
  memset(v, 0, sizeof(*v));
  v->octave = p[0];
  v->note = p[1];
  v->pcmformat = p[2] & 1;
  v->loop = !(p[3] & 0x40);
  v->pcmstart = getaddr(p + 3) & ~0xff;
  v->loopstart = getaddr(p + 5);
  v->loopend = getaddr(p + 8);
  v->pcmend = getaddr(p + 11);
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
void putvoice(struct rx5voice *v, uint8_t *p) {
  memset(p, 0, 32);
  p[0] = v->octave;
  p[1] = v->note;
  p[2] = v->pcmformat & 1;
  putaddr(p + 3, v->pcmstart);
  p[3] |= v->loop ? 0 : 0x40;
  putaddr(p + 5, v->loopstart);
  putaddr(p + 8, v->loopend);
  putaddr(p + 11, v->pcmend);
  p[14] = v->ar;
  p[15] = v->d1r;
  p[16] = v->rr;
  p[17] = v->d2r;
  p[18] = v->d1l;
  p[19] = v->gt;
  p[20] = v->bendrate;
  p[21] = v->bendrange;
  p[22] = v->unknown;
  p[23] = v->level;
  p[24] = v->channel;
  memmove(p + 26, v->name, 6);
}
void loadrom(struct rx5rom *rom, FILE *f) {
  int i;
  if (fread(rom->data, 1, sizeof(rom->data), f) != sizeof(rom->data))
    errx(-1, "ROM too small");
  if (fgetc(f) >= 0)
    errx(-1, "ROM too big");
  if (memcmp(rom->data, "\x00\x00\x00\x00", 4))
    errx(-1, "missing leading zeroes");
  rom->nvoice = rom->data[5];
  for (i = 0; i < rom->nvoice; i++)
    loadvoice(rom->voice + i, rom->data + 6 + i * 32);
}
uint16_t checksum(struct rx5rom *rom, int n) {
  uint16_t sum = 0;
  int i;
  for (i = 0; i < n; i++)
    sum += rom->data[i];
  return sum;
}
void storevoices(struct rx5rom *rom) {
  int i, sum;
  rom->data[5] = rom->nvoice;
  for (i = 0; i < rom->nvoice; i++)
    putvoice(rom->voice + i, rom->data + 6 + i * 32);
  sum = checksum(rom, 1022);
  rom->data[4] = (sum & 0xff) ^ (sum >> 8); /* ROM ID */
  /* re-calculate checksum because ROM ID changed it */
  sum = checksum(rom, 1022);
  rom->data[1022] = sum >> 8;
  rom->data[1023] = sum;
}
