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
    printf(fNAME " %6.6s\n", v->name);
    printf(fOCTAVE " %d\n", v->octave);
    printf(fNOTE " %d\n", v->note);
    printf("pcmformat %s\n", v->pcmformat ? "12-bit" : "8-bit");
    printf(fLOOP " %d\n", v->loop);
    printf(fPCMSTART " 0x%x\n", v->pcmstart);
    printf(fLOOPSTART " 0x%x\n", v->loopstart);
    printf(fLOOPEND " 0x%x\n", v->loopend);
    printf(fPCMEND " 0x%x\n", v->pcmend);
    printf(fATTACKRATE " %d\n" fDECAY1RATE " %d\n" fDECAY1LEVEL
                       " %d\n" fDECAY2RATE " %d\n" fRELEASERATE
                       " %d\n" fGATETIME " %d\n",
           v->ar, v->d1r, v->d1l, v->d2r, v->rr, v->gt);
    printf(fBENDRATE " %d\n" fBENDRANGE " %d\n", v->bendrate, v->bendrange);
    printf(fUNKNOWN " %d\n", v->unknown);
    printf(fLEVEL " %d\n", v->level);
    printf(fCHANNEL " %d\n", v->channel);
  }
  return 0;
}
