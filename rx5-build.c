/* rx5-build: build RX5 ROM out of WAV files */
#include <err.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define assert(x)                                                              \
  if (!(x))                                                                    \
  __builtin_trap()
uint8_t rom[128 * 1024], wav[(sizeof(rom) * 2) / 3 * 4];
uint64_t getle(uint8_t *p, int size) {
  uint64_t x = 0;
  assert(size > 0 && size < 9);
  p += size;
  while (size--)
    x = (x << 8) | *--p;
  return x;
}
void putwav(FILE *f, int channel, char *filename) {
  int wavsize;
  uint8_t *p, *wavend, *fmt;
  uint64_t x;
  if (wavsize = fread(wav, 1, sizeof(wav), f), wavsize == sizeof(wav))
    errx(-1, "WAV file too big: %s", filename);
  if (wavsize < 12)
    errx(-1, "WAV file too small: %s", filename);
  p = wav;
  wavend = wav + wavsize;
  if (memcmp(p, "RIFF", 4))
    errx(-1, "missing RIFF header");
  p += 4;
  if (x = getle(p, 4), x != wavsize - 8)
    errx(-1, "WAV file size does not match RIFF header: %llu: %s", x, filename);
  p += 4;
  if (memcmp(p, "WAVE", 4))
    errx(-1, "missing WAVE header");
  for (p = wav + 12; p < wavend - 8 && memcmp(p, "fmt ", 4);
       p += 8 + getle(p + 4, 4))
    ;
  if (x = getle(p + 4, 4), x != 16)
    errx(-1, "unsupported WAV fmt size: %llu:: %s", x, filename);
  fmt = p + 8;
  if (x = getle(fmt, 2), x != 1)
    errx(-1, "unsupported WAV format: %llu:: %s", x, filename);
  if (x = getle(fmt + 2, 2), x != 1)
    errx(-1, "unsupported number of channels: %llu: %s", x, filename);
  if (x = getle(fmt + 4, 4), x != 25000)
    warnx("wrong samplerate: %llu: %s", x, filename);
  for (p = wav + 12; p < wavend - 8 && memcmp(p, "data", 4);
       p += 8 + getle(p + 4, 4))
    ;
}
int main(int argc, char **argv) {
  int i;
  if (argc < 3 || !(argc & 1))
    errx(-1, "Usage: rx5-build CHANNEL WAV [CHANNEL WAV...]");
  for (i = 1; i < argc; i += 2) {
    FILE *f;
    int channel = atoi(argv[i]);
    if (channel < 1 || channel > 12)
      errx(-1, "invalid channel: %s", argv[i]);
    if (f = fopen(argv[i + 1], "rb"), !f)
      err(-1, "open %s", argv[i + 1]);
    putwav(f, channel, argv[i + 1]);
    fclose(f);
  }
  return 0;
}
