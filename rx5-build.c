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
#define endof(x) (x + nelem(x))
#define assert(x)                                                              \
  if (!(x))                                                                    \
  __builtin_trap()
#define NCHAN 12
struct rx5rom rom;
uint8_t wav[(sizeof(rom.data) * 2) / 3 * 4];
int nvoices;
uint64_t getle(uint8_t *p, int size) {
  uint64_t x = 0;
  while (size--)
    x = (x << 8) | p[size];
  return x;
}
#define CHUNK_HEADER 8
uint8_t *findchunk(char *ID, uint8_t *start, uint8_t *end) {
  uint8_t *p = start;
  while (p < end - CHUNK_HEADER) {
    uint32_t size = getle(p + 4, 4);
    if (size > end - (p + CHUNK_HEADER))
      errx(-1, "chunk %4.4s: invalid size %d", p, size);
    if (!memcmp(p, ID, 4))
      return p;
    p += CHUNK_HEADER + (uint64_t)size + (size & 1);
  }
  return end;
}
void putname(struct rx5voice *v, char *s) {
  char *p;
  for (p = v->name; p < endof(v->name); p++)
    *p = *s ? *s++ : ' ';
}
/* getword sums multi-channel audio to mono */
uint64_t getword(uint8_t *p, struct wavfmt *fmt) {
  uint64_t x = 0;
  int i, samplebytes = fmt->blockalign / fmt->channels;
  for (i = 0; i < fmt->channels; i++)
    x += getle(p + i * samplebytes, samplebytes);
  return x / fmt->channels;
}
uint64_t resize(uint64_t word, int from, int to) {
  assert(from > 0 && to > 0);
  return (from > to) ? word >> (from - to) : word << (to - from);
}
#define NOSPACE "not enough space in ROM for sample"
void putwav(FILE *f, char *filename, int pcmformat) {
  int64_t wavsize, datasize;
  uint8_t *p, *wavend, *data;
  struct rx5voice *voice = rom.voice + rom.nvoice++,
                  defaultvoice = {"      ", 2,  120, 0,  0,  0, 0, 0,  0,  99,
                                  2,        60, 99,  59, 92, 0, 0, 99, 27, 0};
  struct wavfmt fmt;
  uint64_t x;
  int type, wordsize, firstvoice = voice == rom.voice;
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
  if (loadfmt(p, &fmt))
    errx(-1, "invalid WAV fmt chunk");
  if (type = wavtype(&fmt), type != 1)
    errx(-1, "unsupported WAV format: %d", type);
  if (fmt.channels < 1)
    errx(-1, "unsupported number of channels: %d", fmt.channels);
  if (fmt.samplespersec != 25000)
    warnx("warning: samplerate is not 25kHz: %d", fmt.samplespersec);
  if (fmt.blockalign < 1)
    errx(-1, "invalid fmt.blockalign: %d", fmt.blockalign);
  if (p = findchunk("data", wav + 12, wavend), p == wavend)
    errx(-1, "WAV data section not found");
  data = p + 8;
  datasize = getle(p + 4, 4);
  wordsize = (8 * fmt.blockalign) / fmt.channels;
  *voice = defaultvoice;
  putname(voice, filename);
  /* Round starting point up from last sample and ensure there is some empty
   * space. If we do not end the new sample starts right after the previous one
   * the previous one gets a click at the end. */
  voice->pcmstart =
      firstvoice ? 0x400 : ((voice - 1)->pcmend + 0x103) & 0x1ff00;
  voice->loopstart = voice->pcmstart;
  voice->pcmformat = pcmformat;
  voice->channel = firstvoice ? 0 : ((voice - 1)->channel + 1) % NCHAN;
  if (voice->pcmformat) { /* store 12-bit sample */
    uint8_t *q = rom.data + voice->pcmstart + 2;
    for (p = data; p < data + datasize; p += fmt.blockalign) {
      uint16_t word = resize(getword(p, &fmt), wordsize, 12);
      if (q >= endof(rom.data))
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
    if (((voice->pcmend - (voice->pcmstart + 1)) % 3) == 1) {
      /* pcmend points at even word */
      voice->pcmend = 0x100000 | (voice->pcmend - 1);
    }
  } else { /* store 8-bit sample */
    uint8_t *q = rom.data + voice->pcmstart;
    for (p = data; p < data + datasize; p += fmt.blockalign) {
      uint8_t word = resize(getword(p, &fmt), wordsize, 8);
      if (q >= endof(rom.data))
        errx(-1, NOSPACE);
      *q++ = word;
    }
    voice->pcmend = q - rom.data;
  }
  voice->loopend = voice->pcmend;
}
char *matchfield(char *s, char *field) {
  char *tail = s + strlen(field);
  return strstr(s, field) == s && *tail == ' ' ? tail + 1 : 0;
}
int32_t readaddr(char *s) {
  int x = atoi(s);
  if (x < 0 || (x & 0x1ffff) > sizeof(rom.data))
    errx(-1, "invalid %s address: %d", s, x);
  return x;
}
char line[1024];
int main(void) {
  int32_t romid = -1;
  while (fgets(line, sizeof(line), stdin)) {
    struct rx5voice *v = rom.voice + rom.nvoice - 1;
    char *s, *eol = strchr(line, '\n');
    if (!eol)
      errx(-1, "missing newline in input");
    *eol = 0;
    if (*line == '#' || matchfield(line, fPCMSTART) ||
        matchfield(line, fPCMEND))
      continue;
    if (rom.nvoice >= nelem(rom.voice))
      errx(-1, "too many voice entries");
    if (s = matchfield(line, "romid"), s) {
      int x = atoi(s);
      if (rom.nvoice)
        errx(-1, "romid after file statement");
      if (x < 0 || x > 255)
        errx(-1, "invalid romid: %d", x);
      romid = x;
    } else if ((s = matchfield(line, "file8")) ||
               (s = matchfield(line, "file12"))) {
      FILE *f = fopen(s, "rb");
      if (!f)
        err(-1, "%s", s);
      warnx("adding %s", s);
      putwav(f, s, s[-2] == '2');
      fclose(f);
    } else if (!strcmp(line, "copy")) {
      if (!rom.nvoice)
        errx(-1, "copy before file statement");
      warnx("copying last voice");
      v = rom.voice + rom.nvoice++;
      *v = *(v - 1);
      v->channel = ((v - 1)->channel + 1) % NCHAN;
    } else if (s = matchfield(line, fNAME), s) {
      if (!rom.nvoice)
        errx(-1, "name before file statement");
      putname(v, s);
    } else if (s = matchfield(line, fLOOPSTART), s) {
      v->loopstart = readaddr(s);
    } else if (s = matchfield(line, fLOOPEND), s) {
      v->loopend = readaddr(s);
    } else {
      struct {
        ptrdiff_t offset;
        char *field;
        int min, max;
      } params[] =
          {
              {offsetof(struct rx5voice, octave), fOCTAVE, 0, 4},
              {offsetof(struct rx5voice, note), fNOTE, 0, 120},
              {offsetof(struct rx5voice, loop), fLOOP, 0, 1},
              {offsetof(struct rx5voice, ar), fATTACKRATE, 0, 99},
              {offsetof(struct rx5voice, d1r), fDECAY1RATE, 0, 99},
              {offsetof(struct rx5voice, d1l), fDECAY1LEVEL, 0, 60},
              {offsetof(struct rx5voice, d2r), fDECAY2RATE, 0, 99},
              {offsetof(struct rx5voice, rr), fRELEASERATE, 0, 99},
              {offsetof(struct rx5voice, gt), fGATETIME, 0, 255},
              {offsetof(struct rx5voice, bendrate), fBENDRATE, 0, 255},
              {offsetof(struct rx5voice, bendrange), fBENDRANGE, 0, 255},
              {offsetof(struct rx5voice, reverseattackrate), fREVERSEATTACKRATE,
               0, 99},
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
          ((uint8_t *)v)[pp->offset] = x;
          break;
        }
      }
      if (pp == params + nelem(params))
        errx(-1, "invalid statement: %s", line);
    }
  }
  storevoices(&rom, romid);
  warnx("PCM data space left: %lu bytes",
        (sizeof(rom.data) -
         (rom.nvoice ? rom.voice[rom.nvoice - 1].pcmend & 0x1ffff : 0x400)) &
            ~0xff);
  return !fwrite(rom.data, sizeof(rom.data), 1, stdout);
}
