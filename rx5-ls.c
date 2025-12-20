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
    printf("### %s voice\n", v->pcmformat ? "12-bit" : "8-bit");
    printvoice(v, stdout);
  }
  return 0;
}
