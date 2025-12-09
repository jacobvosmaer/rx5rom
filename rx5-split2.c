/* rx5-split: extract WAV files from Yamaha RX5 ROM dump */
#include "rx5.h"
#include <err.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#define assert(x)                                                              \
  if (!(x))                                                                    \
  __builtin_trap()
void writewav(uint16_t *pcmdata, int nsamples, int samplerate, FILE *f);
struct rx5rom rom;
uint16_t pcmdata[128 * 1024];
int32_t convertaddr(struct rx5voice *v, int32_t addr) {
  addr &= 0x1ffff;
  assert(addr >= v->pcmstart);
  if (((addr - v->pcmstart) % 3) == 2)
    errx(-1, "unexpected modulus");
  return addr + 2;
}
int convertword12(uint8_t *start, uint8_t *p, uint16_t *dest) {
  int ret = 1;
  assert(p > start);
  if ((p - start) % 3) {
    *dest = (p[0] << 8) | ((p[-1] & 0x0f) << 4);
  } else {
    *dest = (p[0] << 8) | (p[-2] & 0xf0);
    ret++;
  }
  return ret;
}
int main(int argc, char **argv) {
  struct rx5voice *v;
  FILE *f, *txt;
  if (argc != 3)
    errx(-1, "Usage: rx5-split2 FILE DIR");
  if (f = fopen(argv[1], "rb"), !f)
    err(-1, "open %s", argv[1]);
  loadrom(&rom, f);
  fclose(f);
  if (chdir(argv[2]))
    err(-1, "chdir %s", argv[2]);
  if (txt = fopen("rom.txt", "wb"), !txt)
    err(-1, "open rom.txt");
  fprintf(txt, "romid %d\n", rom.data[4]);
  for (v = rom.voice; v < rom.voice + rom.nvoice; v++) {
    int i;
    uint8_t filename[11] = "123456.wav", *p, *pcmstart = rom.data + v->pcmstart,
            *pcmend = rom.data + (v->pcmend & 0x1ffff) + 1;
    uint16_t *q = pcmdata;
    if (v->pcmformat) {
      pcmend = rom.data + convertaddr(v, v->loopend) - 2;
      if (pcmend > rom.data + sizeof(rom.data))
        errx(-1, "invalid pcmend: %d", v->pcmend);
      for (p = rom.data + convertaddr(v, v->pcmstart); p < pcmend;)
        p += convertword12(rom.data + v->pcmstart, p, q++);
      for (i = 0; i < 3; i++)
        for (p = rom.data + convertaddr(v, v->loopstart); p < pcmend;)
          p += convertword12(rom.data + v->pcmstart, p, q++);
    } else {
      if (pcmend > rom.data + sizeof(rom.data))
        errx(-1, "invalid pcmend: %d", v->pcmend);
      for (p = pcmstart; p < pcmend; p++, q++)
        *q = *p << 8;
    }
    memmove(filename, v->name, 6);
    for (p = filename; p < filename + 10; p++)
      if (!*p || strchr(":/", *p))
        *p = '_';
    if (f = fopen((const char *)filename, "wb"), !f)
      err(-1, "open %s", filename);
    writewav(pcmdata, q - pcmdata, 25000, f);
    if (fclose(f))
      err(-1, "close %s", filename);
    if (fprintf(txt, "###\nfile%d %s\n", v->pcmformat ? 12 : 8, filename) < 0)
      err(-1, "write rom.txt");
    printvoice(v, txt);
  }
  if (fclose(txt))
    err(-1, "close rom.txt");
  return 0;
}
