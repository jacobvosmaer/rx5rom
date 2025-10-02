/* rx5-program: program RX5USB board from command line */
#include <err.h>
#include <hidapi.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#define MSGSIZE 64
#define TIMEOUT 1000
uint8_t request[MSGSIZE + 1] = {0, 'R', 'X', '5', 1, '?', '?', 0,
                                0, 0,   0,   0,   0, 2,   0},
                          response[MSGSIZE];
int main(void) {
  int i;
  hid_device *dev;
  fputs("open device: ", stderr);
  while (dev = hid_open(0x6112, 0x5550, 0), !dev)
    ;
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

  for (i = 0; i < 2048; i++) {
    if (fread(request + 1, 1, MSGSIZE, stdin) != MSGSIZE)
      errx(-1, "short read from stdin");
    fputc('.', stderr);
    while (hid_write(dev, request, sizeof(request)) != sizeof(request))
      ;
    fputc('o', stderr);
    if (0) {
      while (hid_read(dev, response, sizeof(response)) != sizeof(response))
        ;
      fputc('O', stderr);
      if (memcmp(request + 1, response, MSGSIZE))
        errx(-1, "I/O error: invalid response from device");
    }
  }
  return 0;
}
