/* rx5-program: program RX5USB board from command line */
#include <err.h>
#include <hidapi.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define MSGSIZE 64
uint8_t request[MSGSIZE + 1] = "\x00RX5\x01", response[MSGSIZE];
#define BANKSIZE (128 * 1024)
uint8_t bank[BANKSIZE + 1];
void putle(uint8_t *p, uint64_t x, int n) {
  for (; n--; x >>= 8)
    *p++ = x;
}
void usage(void) {
  fputs("Usage: rx5-program 1A|1B|2A|2B BANKFILE\n", stderr);
  exit(1);
}
int main(int argc, char **argv) {
  int i, n;
  char *p, *slots = "1A1B2A2B";
  FILE *f;
  hid_device *dev = 0;
  struct hid_device_info *devinfo;
  uint32_t address;
  if (argc != 3)
    usage();
  if (p = strstr(slots, argv[1]),
      strlen(argv[1]) != 2 || !p || ((p - slots) & 1))
    usage();
  address = (p - slots) / 2 * BANKSIZE;
  if (f = fopen(argv[2], "rb"), !f)
    err(-1, "open %s", argv[2]);
  if (n = fread(bank, 1, sizeof(bank), f), n != BANKSIZE)
    errx(-1, "bank file too %s", n < BANKSIZE ? "small" : "big");
  for (devinfo = hid_enumerate(0x6112, 0x5550); !dev && devinfo;
       devinfo = devinfo->next)
    if (devinfo->usage_page == 0xffab && devinfo->usage == 0x200)
      dev = hid_open_path(devinfo->path);
  if (!dev)
    errx(-1, "failed to open usb device");
  putle(request + 1 + 4, address, 4);
  putle(request + 1 + 8, BANKSIZE, 4);
  if (hid_write(dev, request, sizeof(request)) != sizeof(request))
    errx(-1, "hid_write handshake failed");
  if (hid_read(dev, response, sizeof(response)) != sizeof(response))
    errx(-1, "hid_read handshake failed");
  memmove(request + 1 + 12, "OK", 2);
  if (memcmp(request + 1, response, MSGSIZE))
    errx(-1, "invalid handshake response");
  fprintf(stderr, "write %d bytes to address 0x%x: ", BANKSIZE, address);
  for (i = 0; i < BANKSIZE / MSGSIZE; i++) {
    memmove(request + 1, bank + i * MSGSIZE, MSGSIZE);
    if (hid_write(dev, request, sizeof(request)) != sizeof(request))
      errx(-1, "hid_write block %d failed", i);
    if (hid_read(dev, response, sizeof(response)) != sizeof(response))
      errx(-1, "hid_read block %d failed", i);
    if (memcmp(request + 1, response, MSGSIZE))
      errx(-1, "read back of block %d failed", i);
    if (i > 0 && !(i % 64))
      fputc('.', stderr);
  }
  fputs(" ok\n", stderr);
  hid_close(dev);
  return 0;
}
