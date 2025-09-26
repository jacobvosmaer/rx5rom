/* rx5-ls: list contents of Yamaha RX5 ROM dump */
#include <err.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
uint8_t rom[128 * 1024];
int32_t int24(uint8_t *p) {
  return (((int32_t)p[0] & 1) << 16) | ((int32_t)p[1] << 8) | (int32_t)p[2];
}
int main(void) {
  int i, nvoices;
  if (fread(rom, 1, sizeof(rom), stdin) != sizeof(rom))
    errx(-1, "ROM too small");
  if (fgetc(stdin) >= 0)
    errx(-1, "ROM too big");
  if (memcmp(rom, "\x00\x00\x00\x00", 4))
    errx(-1, "missing leading zeroes");
  printf("ROM ID: %d\n", rom[4]);
  nvoices = rom[5];
  printf("Number of voices: %d\n", nvoices);
  for (i = 0; i < nvoices; i++) {
    uint8_t *p = rom + 6 + 32 * i;
    int32_t pcmstart, loopstart, loopend;
    puts("---");
    printf("Voice name: %6.6s\n", p + 26);
    printf("Pitch: octave %d note %d\n", p[0], p[1]);
    printf("PCM format: %s\n", p[2] ? "12-bit" : "8-bit");
    printf("Loop: %s\n", !(p[3] & 0x40) ? "yes" : "no");
    pcmstart = ((int)(p[3] & 1) << 16) | ((int)(p[4]) << 8);
    printf("PCM data start: %#x\n", pcmstart);
    loopstart = int24(p + 5);
    printf("Loop start: %#x\n", loopstart);
    loopend = int24(p + 8);
    printf("Loop end: %#x\n", loopend);
  }
  return 0;
}
