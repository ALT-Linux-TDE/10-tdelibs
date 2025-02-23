#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "sha1.h"

void printHex (const unsigned char *data) {
     for (int i = 0; i < 20; i++) {
        printf("%.2X", *data++);
        if (i>0 && (i-1)%2 == 0) printf(" ");
     }
     printf("\n");
}

int main() {
  SHA1 *sha1;
  const unsigned char data[] = "abcdbcdecdefdefgefghfghighijhijkijkljklmklmnlmnomnopnopq";
  const unsigned char expected[20] = {
      0x84, 0x98, 0x3e, 0x44,
      0x1c, 0x3b, 0xd2, 0x6e,
      0xba, 0xae, 0x4a, 0xa1,
      0xf9, 0x51, 0x29, 0xe5,
      0xe5, 0x46, 0x70, 0xf1
  };

  unsigned long et[] = {0x11223344};
  int rc;

  printf("%d:  0x11 == %d and 0x44 == %d\n", ((unsigned char *)et)[0],
                                             0x11, 0x44);
  sha1 = new SHA1();

  if (!sha1->readyToGo()) {
     printf("Error: not ready to go!\n");
     return -1;
  }

  printf("About to process [%s]\n", data);
  rc = sha1->process(data, strlen((char *)data));

  if (rc != (int)strlen((char *)data)) {
     printf("Error processing the data.  rc=%d\n", rc);
     return -1;
  } else printf("Done.\n");

  const unsigned char *res = sha1->hash();

  if (res) {
    if (memcmp (res, expected, 20) ==0 ) {
        printf("The result is expected: ");
        printHex (res);
    } else {
        printf("The result is different from expected:\n");
        printf("Result: ");
        printHex (res);
        printf("Expected: ");
        printHex (expected);
        return -1;
    }
  } else {
    printf("Error - hash() returned NULL!\n");
    return -1;
  }

  delete sha1;

  return 0;
}
