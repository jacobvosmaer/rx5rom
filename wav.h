#include <stdint.h>
#include <stdio.h>
void writewav(uint16_t *pcmdata, int nsamples, int samplerate, FILE *f);
struct wavfmt {
  uint16_t formattag, channels;
  uint32_t samplespersec, avgbytespersec;
  uint16_t blockalign, bitspersample, cbsize, validbitspersample;
  uint32_t channelmask;
  uint8_t subformat[16];
};
int loadfmt(uint8_t *p, struct wavfmt *fmt);
int wavtype(struct wavfmt *fmt);
