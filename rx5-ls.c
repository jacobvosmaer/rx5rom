/* rx5-ls: list contents of Yamaha RX5 ROM dump */
#include <err.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
int32_t int24(uint8_t *p) {
  return (((int32_t)p[0] & 1) << 16) | ((int32_t)p[1] << 8) | (int32_t)p[2];
}
struct rx5voice {
  char name[7];
  uint8_t octave, note, pcmformat, loop;
  int32_t pcmstart, pcmend, loopstart, loopend;
  uint8_t ar, d1r, rr, d2r, d1l, gt;
  uint8_t bendrate, bendrange, unknown, level, channel;
};
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
struct rx5rom {
  struct rx5voice voice[256];
  int nvoice;
  uint8_t data[128 * 1024];
};
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
struct rx5rom rom;
int main(int argc, char **argv) {
  struct rx5voice *v;
  FILE *f = stdin;
  if (argc == 2 && strcmp(argv[1], "-")) {
    if (f = fopen(argv[1], "rb"), !f)
      err(-1, "open %s", argv[1]);
  } else {
    errx(-1, "Usage: rx5-ls FILE|-");
  }
  loadrom(&rom, f);
  printf("ROM ID: %d\n", rom.data[4]);
  printf("Number of voices: %d\n", rom.nvoice);
  for (v = rom.voice; v < rom.voice + rom.nvoice; v++) {
    puts("---");
    printf("Voice name: %6.6s\n", v->name);
    printf("Pitch: octave %d note %d\n", v->octave, v->note);
    printf("PCM format: %s\n", v->pcmformat ? "12-bit" : "8-bit");
    printf("Loop: %s\n", v->loop ? "yes" : "no");
    printf("PCM data start: %#x\n", v->pcmstart);
    printf("Loop start: %#x\n", v->loopstart);
    printf("Loop end: %#x\n", v->loopend);
    printf("PCM data end: %#x\n", v->pcmend);
    printf("Envelope: AR %02d D1R %02d  D1L %02d D2R %02d RR %02d GT %d\n",
           v->ar, v->d1r, v->d1l, v->d2r, v->rr, v->gt);
    printf("Bend: rate %d range %d\n", v->bendrate, v->bendrange);
    printf("Unknown: %d\n", v->unknown);
    printf("Level: %d\n", v->level);
    printf("Channel: %d\n", v->channel + 1);
  }
  return 0;
}
