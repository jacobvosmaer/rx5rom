

#include <stdint.h>
#include <stdio.h>
int main(void) {
  int c;
  uint16_t sum = 0;
  while (c = getchar(), c >= 0)
    sum += c;
  printf("%04x\n", sum);
  return 0;
}
