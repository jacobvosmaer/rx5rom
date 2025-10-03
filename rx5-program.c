/* rx5-program: program RX5USB board from command line */
#include <err.h>
#include <hidapi.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#define MSGSIZE 64
uint8_t request[MSGSIZE + 1] = "\x00RX5\x01", response[MSGSIZE];
void putle(uint8_t *p, uint64_t x, int n) {
  for (; n--; x >>= 8)
    *p++ = x;
}
int main(void) {
  int i;
  hid_device *dev = 0;
  struct hid_device_info *devinfo;
  uint32_t address = 0, size = 2 * 128 * 1024;
  putle(request + 1 + 4, address, 4);
  putle(request + 1 + 8, size, 4);
  fputs("open device: ", stderr);
  for (devinfo = hid_enumerate(0x6112, 0x5550); devinfo;
       devinfo = devinfo->next) {
    if (devinfo->usage_page == 0xffab && devinfo->usage == 0x200) {
      dev = hid_open_path(devinfo->path);
      break;
    }
  }
  if (!dev)
    errx(-1, "failed to open usb device");
  fputs("ok\n", stderr);
  fputs("write handshake: ", stderr);
  if (hid_write(dev, request, sizeof(request)) != sizeof(request))
    errx(-1, "hid_write handshake failed");
  fputs("ok\n", stderr);
  fputs("read handshake: ", stderr);
  if (hid_read(dev, response, sizeof(response)) != sizeof(response))
    errx(-1, "hid_read handshake failed");
  fputs("ok\n", stderr);
  request[1 + 12] = 'O';
  request[1 + 13] = 'K';
  if (memcmp(request + 1, response, MSGSIZE))
    errx(-1, "invalid handshake response");
  fprintf(stderr, "write %d bytes to address 0x%x: ", size, address);
  for (i = 0; i < size / 64; i++) {
    if (fread(request + 1, 1, MSGSIZE, stdin) != MSGSIZE)
      errx(-1, "short read from stdin");
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
