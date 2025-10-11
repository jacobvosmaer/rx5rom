#include <stdint.h>
#include <stdio.h>
struct rx5voice {
  char name[7];
  uint8_t octave, note, pcmformat, loop;
  int32_t pcmstart, pcmend, loopstart, loopend;
  uint8_t ar, d1r, rr, d2r, d1l, gt;
  uint8_t bendrate, bendrange, unknown, level, channel;
};
struct rx5rom {
  struct rx5voice voice[256];
  int nvoice;
  uint8_t data[128 * 1024];
};
void loadrom(struct rx5rom *rom, FILE *f);
void storevoices(struct rx5rom *rom);
#define fNAME "name"
#define fOCTAVE "octave"
#define fNOTE "note"
#define fATTACKRATE "attackrate"
#define fDECAY1RATE "decay1rate"
#define fDECAY1LEVEL "decay1level"
#define fDECAY2RATE "decay2rate"
#define fRELEASERATE "releaserate"
#define fGATETIME "gatetime"
#define fBENDRATE "bendrate"
#define fBENDRANGE "bendrange"
#define fUNKNOWN "unknown"
#define fLEVEL "level"
#define fCHANNEL "channel"
