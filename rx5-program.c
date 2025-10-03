/* rx5-program: program RX5USB board from command line */
#include <err.h>
#include <hidapi.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#define BANKSIZE (128 * 1024)
#define MSGSIZE 64
uint8_t request[MSGSIZE + 1] = "\x00RX5\x01", response[MSGSIZE];
void putle(uint8_t *p, uint64_t x, int n) {
  for (; n--; x >>= 8)
    *p++ = x;
}
int main(int argc, char **argv) {
  int i;
  hid_device *dev = 0;
  struct hid_device_info *devinfo;
  uint32_t address, size;
  if (argc != 3 || strlen(argv[1]) != 1 || strlen(argv[2]) != 1)
    errx(-1, "usage: rx5-program FIRST_SLOT NUM_BANKS");
  if (address = argv[1][0] - '0', address < 0 || address > 3)
    errx(-1, "invalid first slot: %s", argv[1]);
  if (size = argv[2][0] - '0', size < 0 || address + size > 4)
    errx(-1, "invalid number of banks: %s", argv[2]);
  address *= BANKSIZE;
  size *= BANKSIZE;
  for (devinfo = hid_enumerate(0x6112, 0x5550); !dev && devinfo;
       devinfo = devinfo->next)
    if (devinfo->usage_page == 0xffab && devinfo->usage == 0x200)
      dev = hid_open_path(devinfo->path);
  if (!dev)
    errx(-1, "failed to open usb device");
  putle(request + 1 + 4, address, 4);
  putle(request + 1 + 8, size, 4);
  if (hid_write(dev, request, sizeof(request)) != sizeof(request))
    errx(-1, "hid_write handshake failed");
  if (hid_read(dev, response, sizeof(response)) != sizeof(response))
    errx(-1, "hid_read handshake failed");
  memmove(request + 1 + 12, "OK", 2);
  if (memcmp(request + 1, response, MSGSIZE))
    errx(-1, "invalid handshake response");
  fprintf(stderr, "write %d bytes to address 0x%x: ", size, address);
  for (i = 0; i < size / MSGSIZE; i++) {
    if (fread(request + 1, 1, MSGSIZE, stdin) != MSGSIZE)
      errx(-1, "bank file too small");
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
