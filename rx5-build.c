/* rx5-build: build RX5 ROM out of WAV files */
#include "rx5.h"
#include "wav.h"
#include <err.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifndef offsetof
#define offsetof(t, f) (size_t)((char *)&((t *)0)->f - (char *)0)
#endif
#define nelem(x) (sizeof(x) / sizeof(*(x)))
#define assert(x)                                                              \
  if (!(x))                                                                    \
  __builtin_trap()
typedef uint8_t u8;
typedef uint32_t u32;
typedef uint64_t u64;
typedef int64_t i64;
struct rx5rom rom;
u8 wav[(sizeof(rom.data) * 2) / 3 * 4];
int nvoices;
u64 getle(u8 *p, int size) {
  u64 x = 0;
  while (size--)
    x = (x << 8) | p[size];
  return x;
}
#define CHUNK_HEADER 8
u8 *findchunk(char *ID, u8 *start, u8 *end) {
  u8 *p = start;
  while (p < end - CHUNK_HEADER) {
    u32 size = getle(p + 4, 4);
    if (size > end - (p + CHUNK_HEADER))
      errx(-1, "chunk %4.4s: invalid size %d", p, size);
    if (!memcmp(p, ID, 4))
      return p;
    p += CHUNK_HEADER + (u64)size + (size & 1);
  }
  return end;
}
void putname(char *dst, char *s) {
  char *p;
  for (p = dst; p < dst + 6; p++)
    *p = *s ? *s++ : ' ';
  *p = 0;
}
#define NOSPACE "not enough space in ROM for sample"
void putwav(FILE *f, char *filename) {
  i64 wavsize, datasize;
  u8 *p, *wavend, *data;
  struct rx5voice *voice = rom.voice + rom.nvoice++,
                  defaultvoice = {"      ", 2,  120, 0,  0,  0, 0, 0,  0,  99,
                                  2,        60, 99,  59, 92, 0, 0, 99, 27, 0};
  struct wavfmt fmt;
  u64 x;
  int wordsize, firstvoice = voice == rom.voice;
  if (wavsize = fread(wav, 1, sizeof(wav), f), wavsize == sizeof(wav))
    errx(-1, "WAV file too big");
  if (wavsize < 12)
    errx(-1, "WAV file too small");
  wavend = wav + wavsize;
  if (memcmp(wav, "RIFF", 4))
    errx(-1, "missing RIFF header");
  if (x = getle(wav + 4, 4), x != wavsize - 8)
    errx(-1, "WAV file size does not match RIFF header: %llu", x);
  if (memcmp(wav + 8, "WAVE", 4))
    errx(-1, "missing WAVE header");
  if (p = findchunk("fmt ", wav + 12, wavend), p == wavend)
    errx(-1, "fmt chunk not found");
  if (x = getle(p + 4, 4), x < 14)
    errx(-1, "unsupported WAV fmt size: %llu", x);
  fmt = loadfmt(p);
  if (fmt.formattag != 1)
    errx(-1, "unsupported WAV format: %d", fmt.formattag);
  if (fmt.channels != 1)
    errx(-1, "unsupported number of channels: %d", fmt.channels);
  if (fmt.samplespersec != 25000)
    warnx("warning: samplerate is not 25kHz: %d", fmt.samplespersec);
  if (!fmt.blockalign)
    errx(-1, "invalid fmt.blockalign: %d", fmt.blockalign);
  if (p = findchunk("data", wav + 12, wavend), p == wavend)
    errx(-1, "WAV data section not found");
  data = p + 8;
  datasize = getle(p + 4, 4);
  wordsize = (8 * fmt.blockalign) / fmt.channels;
  assert(wordsize >= 8);
  *voice = defaultvoice;
  putname(voice->name, filename);
  voice->pcmstart = firstvoice ? 0x400 : ((voice - 1)->pcmend + 0xff) & ~0xff;
  voice->loopstart = voice->loopend = voice->pcmstart;
  voice->pcmformat = wordsize > 8;
  voice->channel = firstvoice ? 0 : ((voice - 1)->channel + 1) % 12;
  if (voice->pcmformat) { /* store 12-bit sample */
    u8 *q = rom.data + voice->pcmstart + 2;
    for (p = data; p < data + datasize; p += fmt.blockalign) {
      u64 word = getle(p, fmt.blockalign) >> (wordsize - 12);
      if (q >= rom.data + sizeof(rom.data))
        errx(-1, NOSPACE);
      q[0] = word >> 4;
      if (((p - data) / fmt.blockalign) & 1) { /* odd sample */
        q[-2] |= word << 4;
        q += 2;
      } else { /* even sample */
        q[-1] = word & 0xf;
        q += 1;
      }
    }
    voice->pcmend = q - rom.data;
  } else { /* store 8-bit sample */
    if (datasize > sizeof(rom.data) - voice->pcmstart)
      errx(-1, NOSPACE);
    memmove(rom.data + voice->pcmstart, data, datasize);
    voice->pcmend = voice->pcmstart + datasize;
  }
}
char *matchfield(char *s, char *field) {
  char *tail = s + strlen(field);
  return strstr(s, field) == s && *tail == ' ' ? tail + 1 : 0;
}
char line[1024];
int main(void) {
  while (fgets(line, sizeof(line), stdin)) {
    struct rx5voice *v = rom.voice + rom.nvoice - 1;
    char *s, *eol = strchr(line, '\n');
    if (!eol)
      errx(-1, "missing newline in input");
    *eol = 0;
    if (*line == '#')
      continue;
    if (s = matchfield(line, "file"), s) {
      FILE *f = fopen(s, "rb");
      if (!f)
        err(-1, "%s", s);
      warnx("adding %s", s);
      putwav(f, s);
      fclose(f);
    } else if (s = matchfield(line, fNAME), s) {
      if (!rom.nvoice)
        errx(-1, "name before file statement");
      putname(v->name, s);
    } else {
      struct {
        ptrdiff_t offset;
        char *field;
        int min, max;
      } params[] =
          {
              {offsetof(struct rx5voice, octave), fOCTAVE, 0, 4},
              {offsetof(struct rx5voice, note), fNOTE, 0, 120},
              {offsetof(struct rx5voice, ar), fATTACKRATE, 0, 99},
              {offsetof(struct rx5voice, d1r), fDECAY1RATE, 0, 99},
              {offsetof(struct rx5voice, d1l), fDECAY1LEVEL, 0, 60},
              {offsetof(struct rx5voice, d2r), fDECAY2RATE, 0, 99},
              {offsetof(struct rx5voice, rr), fRELEASERATE, 0, 99},
              {offsetof(struct rx5voice, gt), fGATETIME, 0, 255},
              {offsetof(struct rx5voice, bendrate), fBENDRATE, 0, 255},
              {offsetof(struct rx5voice, bendrange), fBENDRANGE, 0, 255},
              {offsetof(struct rx5voice, unknown), fUNKNOWN, 0, 255},
              {offsetof(struct rx5voice, level), fLEVEL, 0, 31},
              {offsetof(struct rx5voice, channel), fCHANNEL, 0, 11},
          },
        *pp;
      for (pp = params; pp < params + nelem(params); pp++) {
        if (s = matchfield(line, pp->field), s) {
          int x = atoi(s);
          if (!rom.nvoice)
            errx(-1, "%s before file statement", pp->field);
          if (x < pp->min || x > pp->max)
            errx(-1, "%s out of range: %s", pp->field, s);
          ((u8 *)v)[pp->offset] = x;
          break;
        }
      }
      if (pp == params + nelem(params))
        errx(-1, "invalid statement: %s", line);
    }
  }
  storevoices(&rom);
  warnx("PCM data space left: %lu bytes",
        (sizeof(rom.data) -
         (rom.nvoice ? rom.voice[rom.nvoice - 1].pcmend : 0x400)) &
            ~0xff);
  return !fwrite(rom.data, sizeof(rom.data), 1, stdout);
}
