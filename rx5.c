/* rx5.c: library functions for working with Yamaha RX5 rom dumps */
#include "rx5.h"
#include <err.h>
#include <string.h>
int32_t getaddr(uint8_t *p) {
  return ((int32_t)p[0] << 16) | ((int32_t)p[1] << 8) | (int32_t)p[2];
}
void putaddr(uint8_t *p, int32_t x) {
  p[0] = x >> 16;
  p[1] = x >> 8;
  p[2] = x;
}
void loadvoice(struct rx5voice *v, uint8_t *p) {
  memset(v, 0, sizeof(*v));
  v->octave = p[0];
  v->note = p[1];
  v->pcmformat = p[2] & 1;
  v->loop = !(p[3] & 0x40);
  v->pcmstart = getaddr(p + 3) & 0x1ff00;
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
  v->reverseattackrate = p[22];
  v->level = p[23];
  v->channel = p[24];
  memmove(v->name, p + 26, sizeof(v->name));
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
  p[22] = v->reverseattackrate;
  p[23] = v->level;
  p[24] = v->channel;
  memmove(p + 26, v->name, sizeof(v->name));
}
void printvoice(struct rx5voice *v, FILE *f) {
  fprintf(f, fNAME " %6.6s\n", v->name);
  fprintf(f, fOCTAVE " %d\n", v->octave);
  fprintf(f, fNOTE " %d\n", v->note);
  fprintf(f, fLOOP " %d\n", v->loop);
  fprintf(f, fPCMSTART " %d\n", v->pcmstart);
  fprintf(f, fLOOPSTART " %d\n", v->loopstart);
  fprintf(f, fLOOPEND " %d\n", v->loopend);
  fprintf(f, fPCMEND " %d\n", v->pcmend);
  fprintf(f, fATTACKRATE " %d\n", v->ar);
  fprintf(f, fDECAY1RATE " %d\n", v->d1r);
  fprintf(f, fDECAY1LEVEL " %d\n", v->d1l);
  fprintf(f, fDECAY2RATE " %d\n", v->d2r);
  fprintf(f, fRELEASERATE " %d\n", v->rr);
  fprintf(f, fGATETIME " %d\n", v->gt);
  fprintf(f, fBENDRATE " %d\n", v->bendrate);
  fprintf(f, fBENDRANGE " %d\n", v->bendrange);
  fprintf(f, fREVERSEATTACKRATE " %d\n", v->reverseattackrate);
  fprintf(f, fLEVEL " %d\n", v->level);
  fprintf(f, fCHANNEL " %d\n", v->channel);
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
uint16_t checksum(uint8_t *data, int n) {
  uint16_t sum = 0;
  int i;
  for (i = 0; i < n; i++)
    sum += data[i];
  return sum;
}
void storevoices(struct rx5rom *rom, int id) {
  int i, sum;
  rom->data[5] = rom->nvoice;
  for (i = 0; i < rom->nvoice; i++)
    putvoice(rom->voice + i, rom->data + 6 + i * 32);
  sum = checksum(rom->data, 1022);
  rom->data[4] = id < 0 ? (sum & 0xff) ^ (sum >> 8) : id;
  /* re-calculate checksum because ROM ID may have changed it */
  sum = checksum(rom->data, 1022);
  rom->data[1022] = sum >> 8;
  rom->data[1023] = sum;
  /* another checksum, not sure if the RX5 actually reads it but the original
   * cartridges have it */
  sum = checksum(rom->data, sizeof(rom->data) - 2);
  rom->data[sizeof(rom->data) - 2] = sum >> 8;
  rom->data[sizeof(rom->data) - 1] = sum;
}
