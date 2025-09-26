/* rx5-ls: list contents of Yamaha RX5 ROM dump */
#include <err.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
uint8_t rom[128 * 1024];
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
  for (i = 0; i < nvoices; i++)
    printf("Voice name: %6.6s\n", rom + 32 * (i + 1));
  return 0;
}
