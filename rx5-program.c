/* rx5-program: program RX5USB board from command line */
#include <err.h>
#include <hidapi.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#define BANKSIZE (128 * 1024)
#define MSGSIZE 64
typedef hid_device usbdev;
usbdev *usbopen(void) {
  struct hid_device_info *devinfo;
  for (devinfo = hid_enumerate(0x6112, 0x5550); devinfo;
       devinfo = devinfo->next)
    if (devinfo->usage_page == 0xffab && devinfo->usage == 0x200)
      return hid_open_path(devinfo->path);
  return 0;
}
void usbclose(usbdev *dev) { hid_close(dev); }
int usbwrite(usbdev *dev, uint8_t *msg) {
  uint8_t buf[MSGSIZE + 1] = {0};
  memmove(buf + 1, msg, MSGSIZE);
  return hid_write(dev, buf, sizeof(buf)) != sizeof(buf);
}
int usbread(usbdev *dev, uint8_t *msg) {
  return hid_read(dev, msg, MSGSIZE) != MSGSIZE;
}
uint8_t request[MSGSIZE] = "RX5\x01", response[MSGSIZE];
void putle(uint8_t *p, uint64_t x, int n) {
  for (; n--; x >>= 8)
    *p++ = x;
}
int main(int argc, char **argv) {
  int i;
  usbdev *dev;
  uint32_t address, size;
  if (argc != 3 || strlen(argv[1]) != 1 || strlen(argv[2]) != 1)
    errx(-1, "usage: rx5-program FIRST_SLOT NUM_BANKS");
  if (address = argv[1][0] - '0', address < 0 || address > 3)
    errx(-1, "invalid first slot: %s", argv[1]);
  if (size = argv[2][0] - '0', size < 1 || address + size > 4)
    errx(-1, "invalid number of banks: %s", argv[2]);
  address *= BANKSIZE;
  size *= BANKSIZE;
  if (dev = usbopen(), !dev)
    errx(-1, "failed to open usb device");
  putle(request + 4, address, 4);
  putle(request + 8, size, 4);
  if (usbwrite(dev, request))
    errx(-1, "usbwrite handshake failed");
  if (usbread(dev, response))
    errx(-1, "usbread handshake failed");
  memmove(request + 12, "OK", 2);
  if (memcmp(request, response, MSGSIZE))
    errx(-1, "invalid handshake response");
  fprintf(stderr, "write %d bytes to address 0x%x: ", size, address);
  for (i = 0; i < size / MSGSIZE; i++) {
    if (fread(request, 1, MSGSIZE, stdin) != MSGSIZE)
      errx(-1, "bank file too small");
    if (usbwrite(dev, request))
      errx(-1, "usbwrite block %d failed", i);
    if (usbread(dev, response))
      errx(-1, "usbread block %d failed", i);
    if (memcmp(request, response, MSGSIZE))
      errx(-1, "read back of block %d failed", i);
    if (i > 0 && !(i % 64))
      fputc('.', stderr);
  }
  fputs(" ok\n", stderr);
  usbclose(dev);
  return 0;
}
