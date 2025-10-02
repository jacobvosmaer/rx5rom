/* rx5-program: program RX5USB board from command line */
#include "hid.h"
#include <err.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#define MSGSIZE 64
#define TIMEOUT 1000
uint8_t request[MSGSIZE] = {'R', 'X', '5', 1, '?', '?', 0, 0, 0, 0, 0, 0, 2, 0},
        response[MSGSIZE];
int main(void) {
  int i;
  while (rawhid_open(1, 0x6112, 0x5550, 0xffab, 0x0200) != 1)
    ;
  rawhid_send(0, request, sizeof(request), TIMEOUT);
  request[12] = 'O';
  request[13] = 'K';
  while (1) {
    int n = rawhid_recv(0, response, sizeof(response), TIMEOUT);
    if (n == sizeof(response) && !memcmp(request, response, sizeof(request)))
      break;
    else if (n > 0)
      errx(-1, "invalid handshake response");
  }
  puts("handshake ok");
  for (i = 0; i < 2048; i++) {
    if (fread(request, 1, sizeof(request), stdin) != sizeof(request))
      errx(-1, "short read from stdin");
    if (rawhid_send(0, request, sizeof(request), TIMEOUT) != sizeof(request))
      errx(-1, "rawhid_send failed");
    while (rawhid_recv(0, response, sizeof(response), TIMEOUT) !=
           sizeof(response))
      ;
    if (memcmp(request, response, sizeof(request)))
      errx(-1, "I/O error: invalid response from device");
    if ((i % 64) == 63)
      putchar('.');
  }
  return 0;
}
