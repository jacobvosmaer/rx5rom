/* rx5-ls: list contents of Yamaha RX5 ROM dump */
#include "rx5.h"
#include <err.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
void writewav(int16_t *pcmdata, int nsamples, int samplerate, FILE *f);
struct rx5rom rom;
int16_t pcmdata[128 * 1024];
int main(int argc, char **argv) {
  struct rx5voice *v;
  FILE *f;
  if (argc != 3)
    errx(-1, "Usage: rx5-split FILE DIR");
  if (f = fopen(argv[1], "rb"), !f)
    err(-1, "open %s", argv[1]);
  loadrom(&rom, f);
  fclose(f);
  if (chdir(argv[2]))
    err(-1, "chdir %s", argv[2]);
  for (v = rom.voice; v < rom.voice + rom.nvoice; v++) {
    uint8_t filename[11] = "123456.wav", *p, *pcmend = rom.data + v->pcmend;
    int16_t *q = pcmdata;
    if (v->pcmformat) {
      for (p = rom.data + v->pcmstart + 1; p < pcmend; p += 3) {
        *q = ((int16_t)p[0] & 0xf) << 4;
        if (p + 1 < pcmend)
          *q |= (int16_t)p[1] << 8;
        q++;
        if (p + 2 < pcmend)
          *q++ = ((int16_t)p[2] << 8) | ((int16_t)p[0] & 0xf0);
      }
    } else {
      for (p = rom.data + v->pcmstart; p < rom.data + v->pcmend; p++)
        *q++ = (int16_t)*p << 8;
    }
    memmove(filename, v->name, 6);
    for (p = filename; p < filename + 10; p++)
      if (!*p || strchr(":/", *p))
        *p = '_';
    if (f = fopen((const char *)filename, "wb"), !f)
      err(-1, "open %s", filename);
    writewav(pcmdata, q - pcmdata, 25000, f);
    fclose(f);
  }
  return 0;
}
