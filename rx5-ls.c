/* rx5-ls: list contents of Yamaha RX5 ROM dump */
#include "rx5.h"
#include <err.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
struct rx5rom rom;
int main(int argc, char **argv) {
  struct rx5voice *v;
  FILE *f;
  if (argc != 2)
    errx(-1, "Usage: rx5-ls FILE");
  if (f = fopen(argv[1], "rb"), !f)
    err(-1, "open %s", argv[1]);
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
    printf("Channel: %d\n", v->channel);
  }
  return 0;
}
