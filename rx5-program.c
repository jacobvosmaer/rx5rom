/* rx5-program: program RX5USB board from command line */
#include <err.h>
#include <hidapi.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#define MSGSIZE 64
uint8_t request[MSGSIZE + 1] = {0, 'R', 'X', '5', 1, '?', '?'},
                          response[MSGSIZE];
int main(void) {
  int i;
  hid_device *dev = 0;
  struct hid_device_info *devinfo;
  uint32_t size = 2 * 128 * 1024;
  for (i = 0; i < 4; i++)
    request[11 + i] = size >> (i * 8);
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
  while (hid_write(dev, request, sizeof(request)) != sizeof(request))
    ;
  fputs("ok\n", stderr);
  fputs("read handshake: ", stderr);
  while (hid_read(dev, response, sizeof(response)) != sizeof(response))
    ;
  fputs("ok\n", stderr);
  request[13] = 'O';
  request[14] = 'K';
  if (memcmp(request + 1, response, MSGSIZE))
    errx(-1, "invalid handshake response");
  fputs("write flash: ", stderr);
  for (i = 0; i < size / 64; i++) {
    if (fread(request + 1, 1, MSGSIZE, stdin) != MSGSIZE)
      errx(-1, "short read from stdin");
    while (hid_write(dev, request, sizeof(request)) != sizeof(request))
      ;
    if (10) {
      while (hid_read_timeout(dev, response, sizeof(response), 1000) !=
             sizeof(response))
        ;
      if (memcmp(request + 1, response, MSGSIZE))
        errx(-1, "read back of block %d failed", i);
    }
    if (i > 0 && !(i % 64))
      fputc('.', stderr);
  }
  fputs(" ok\n", stderr);
  hid_close(dev);
  return 0;
}
