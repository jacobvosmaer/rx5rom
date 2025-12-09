/* show end of rx5 pcm data */
#include "rx5.h"
#include <err.h>
#include <stdio.h>
struct rx5rom rom;
int main(int argc, char **argv) {
  struct rx5voice *v;
  FILE *f;
  if (argc != 2)
    errx(-1, "Usage: showend FILE");
  if (f = fopen(argv[1], "rb"), !f)
    err(-1, "open %s", argv[1]);
  loadrom(&rom, f);
  puts("                                             v");
  for (v = rom.voice; v < rom.voice + rom.nvoice; v++) {
    int i, pcmend = v->pcmend & 0x1ffff;
    if (!v->pcmformat)
      continue;
    printf("%6.6s %6d %8.2f %c --", v->name, pcmend,
           (pcmend - v->pcmstart - 1) / 3.0, v->pcmend & 0x100000 ? '*' : ' ');
    for (i = pcmend - 4; i < pcmend + 6; i++)
      if (i < sizeof(rom.data))
        printf(" %s%02x", ((i - (v->pcmstart)) % 3) ? " " : ".", rom.data[i]);
    putchar('\n');
  }
  return 0;
}
