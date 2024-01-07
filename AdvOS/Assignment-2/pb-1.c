/*
Simple implementation as mentioned in the xv6 book

1 child producing data and 1 parent consuming data (512 byte chunks at a time)

Approx ticks ~ 120


*/

#include "kernel/types.h"

#include "kernel/stat.h"

#include "user/user.h"


#define BYTES 10 * 1024 * 1024
#define CHUNK 512

int
main(void) {
  int p[2];

  pipe(p);

  // Prepared a  512 byte data
  char data[CHUNK];
  for (int i = 0; i < CHUNK; i++)
    data[i] = 'A';

  if (fork() == 0) {
    close(p[0]);
    // send data to buffer

    // Sending data in 512 byte chunks if possible
    int bytesRem = BYTES;
    while (bytesRem > 0) {
      int sendBytes = (bytesRem < CHUNK) ? bytesRem : CHUNK;
      write(p[1], data, sendBytes);
      bytesRem -= sendBytes;
    }

    close(p[1]);
    exit(0);
  } else {
    int start = uptime();

    // char buf[CHUNK];
    close(0);
    dup(p[0]);
    close(p[0]);
    close(p[1]);

    // read data
    int bytesRead = 0;
    while (bytesRead < BYTES) {
      int readbytes = (BYTES - bytesRead < CHUNK) ? (BYTES - bytesRead) : CHUNK;
      char buf[readbytes];
      int n = read(0, buf, sizeof(buf));
      if (n <= 0) {
        fprintf(1, "Error while reading the pipe");
        exit(1);
      }

      bytesRead += n;
    }
    int elapsed = uptime() - start;

    fprintf(1, "Received %d bytes of data\n", bytesRead);

    fprintf(1, "elapsed is %d\n", elapsed);
    wait((int * ) 0);
    exit(0);
  }

}
