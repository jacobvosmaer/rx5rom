/* rx5-build: build RX5 ROM out of WAV files */
#include "rx5.h"
#include <err.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define assert(x)                                                              \
  if (!(x))                                                                    \
  __builtin_trap()
uint8_t rom[128 * 1024], wav[(sizeof(rom) * 2) / 3 * 4];
struct rx5voice voices[255];
int nvoices;
uint64_t getle(uint8_t *p, int size) {
  uint64_t x = 0;
  assert(size > 0 && size < 9);
  p += size;
  while (size--)
    x = (x << 8) | *--p;
  return x;
}
void putwav(FILE *f, int channel) {
  int wavsize;
  uint8_t *p, *wavend, *fmt;
  struct rx5voice *voice = voices + nvoices++;
  uint64_t x;
  int nchannels, blockalign, samplebits;
  if (wavsize = fread(wav, 1, sizeof(wav), f), wavsize == sizeof(wav))
    errx(-1, "WAV file too big");
  if (wavsize < 12)
    errx(-1, "WAV file too smal");
  p = wav;
  wavend = wav + wavsize;
  if (memcmp(p, "RIFF", 4))
    errx(-1, "missing RIFF header");
  p += 4;
  if (x = getle(p, 4), x != wavsize - 8)
    errx(-1, "WAV file size does not match RIFF header: %llu", x);
  p += 4;
  if (memcmp(p, "WAVE", 4))
    errx(-1, "missing WAVE header");
  for (p = wav + 12; p < wavend - 8 && memcmp(p, "fmt ", 4);
       p += 8 + getle(p + 4, 4))
    ;
  if (p >= wavend - 8 || memcmp(p, "fmt ", 4))
    errx(-1, "fmt chunk not found");
  if (x = getle(p + 4, 4), x != 16)
    errx(-1, "unsupported WAV fmt size: %llu", x);
  fmt = p + 8;
  if (x = getle(fmt, 2), x != 1)
    errx(-1, "unsupported WAV format: %llu", x);
  if (nchannels = getle(fmt + 2, 2), nchannels != 1)
    errx(-1, "unsupported number of channels: %d", nchannels);
  if (x = getle(fmt + 4, 4), x != 25000)
    warnx("warning: samplerate is not 25kHz: %llu", x);
  if (blockalign = getle(fmt + 12, 2), !blockalign)
    errx(-1, "invalid blockalign: %d", blockalign);
  if (samplebits = getle(fmt + 14, 2),
      samplebits != (8 * blockalign) / nchannels)
    errx(-1,
         "bits per sample (%d) does not match blockalign (%d) and channel "
         "count (%d)",
         samplebits, blockalign, nchannels);
  for (p = wav + 12; p < wavend - 8 && getle(p + 4, 4) && memcmp(p, "data", 4);
       p += 8 + getle(p + 4, 4))
    ;
  if (p >= wavend - 8 || memcmp(p, "data", 4))
    errx(-1, "WAV data section not found");
  voice->pcmstart =
      voice > voices ? ((voice - 1)->pcmend + 0xff) & ~0xff : 0x400;
  if (samplebits == 8) {
    /* store 8-bit sample */
    memmove(rom + voice->pcmstart, p + 8, getle(p + 4, 4));
  } else {
    /* store 12-bit sample */
  }
}
int main(int argc, char **argv) {
  int i;
  if (argc < 3 || !(argc & 1))
    errx(-1, "Usage: rx5-build CHANNEL WAV [CHANNEL WAV...]");
  if (argc > 511)
    errx(-1, "too many voices: maximum is 255");
  for (i = 1; i < argc; i += 2) {
    FILE *f;
    int channel = atoi(argv[i]);
    if (channel < 1 || channel > 12)
      errx(-1, "invalid channel: %s", argv[i]);
    if (f = fopen(argv[i + 1], "rb"), !f)
      err(-1, "open %s", argv[i + 1]);
    fprintf(stderr, "%s\n", argv[i + 1]);
    putwav(f, channel);
    fclose(f);
  }
  return 0;
}
