/* wav.c: write 16-bit wav file */
#include "wav.h"
#include <stdint.h>
#include <stdio.h>
#define assert(x)                                                              \
  if (!(x))                                                                    \
  __builtin_trap()
static void putle(unsigned x, int n, FILE *f) {
  for (; n > 0; n--, x >>= 8)
    fputc(x & 0xff, f);
}
static uint64_t getle(uint8_t *p, int size) {
  uint64_t x = 0;
  while (size--)
    x = (x << 8) | p[size];
  return x;
}
/* Based on information from
 * https://www.mmsp.ece.mcgill.ca/Documents/AudioFormats/WAVE/WAVE.html */
void writewav(uint16_t *pcmdata, int nsamples, int samplerate, FILE *f) {
  int samplebits = 16, i;
  int64_t datasize, fmtsize, wavesize, n;
  assert(nsamples >= 0 && nsamples <= UINT32_MAX / 2);
  n = 2 * nsamples;
  datasize = 8 + n;
  fmtsize = 8 + 16;
  wavesize = 4 + fmtsize + datasize;
  assert(wavesize <= UINT32_MAX);
  fputs("RIFF", f);
  putle(wavesize, 4, f);
  fputs("WAVE", f);
  fputs("fmt ", f);
  putle(fmtsize - 8, 4, f); /* fmt chunk size */
  putle(1, 2, f);           /* format 1 (PCM) */
  putle(1, 2, f);           /* 1 channel */
  putle(samplerate, 4, f);
  putle((samplerate * samplebits) / 8, 4, f); /* data rate bytes/s */
  putle(samplebits / 8, 2, f);                /* bytes per sample */
  putle(samplebits, 2, f);                    /* bits per sample */
  fputs("data", f);
  putle(n, 4, f);
  for (i = 0; i < nsamples; i++) {
    fputc(pcmdata[i] & 0xff, f);
    fputc(pcmdata[i] >> 8, f);
  }
}
struct wavfmt loadfmt(uint8_t *p) {
  struct wavfmt fmt = {0};
  fmt.formattag = getle(p + 8, 2);
  fmt.channels = getle(p + 10, 2);
  fmt.samplespersec = getle(p + 12, 4);
  fmt.avgbytespersec = getle(p + 16, 4);
  fmt.blockalign = getle(p + 20, 2);
  return fmt;
}
