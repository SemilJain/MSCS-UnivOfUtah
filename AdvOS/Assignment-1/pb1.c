/*
Modified the previous pb.c to modularize number of children

n children writing data and n children in the parent try to read it from the buffer

Approx ticks for different combo:

1 child - ~120 (similar to pb.c)

3-5 children - ~127
10 children - ~126

15 children - ~124

20 children - ~125


I realized that 1 child and parent has the least ticks as less overhead of system calls switching
*/


#include "kernel/types.h"

#include "kernel/stat.h"

#include "user/user.h"

// Defining some params
#define MB 10 * 1024 * 1024
#define CHUNK 512
#define CHILDREN 20

int
main(void) {
  int p[2];
  pipe(p);

  // Prepared a  512 byte data
  char data[CHUNK];
  for (int i = 0; i < CHUNK; i++)
    data[i] = 'A';

  // Multiple children writing data to pipe
  for (int i = 0; i < CHILDREN; i++) {
    if (fork() == 0) {
      close(p[0]);

      // send data to buffer
      for (int j = 0; j < MB / CHILDREN; j += CHUNK)
        write(p[1], data, CHUNK);

      close(p[1]);
      exit(0);
    }
  }

  char buf[CHUNK];
  close(0);
  dup(p[0]);
  close(p[0]);
  close(p[1]);
  // read data
  int start = uptime();
  int bytesRead = 0;
  for (int i = 0; i < CHILDREN; i++) {
    if (fork() == 0) {
      // Multiple children in Parent Reading data

      while (bytesRead < MB / CHILDREN) {
        int n = read(0, buf, sizeof(buf));
        if (n <= 0)
          exit(1);

        bytesRead += n;
      }
      exit(0);
    }

  }
  int status;
  for (int i = 0; i < CHILDREN * 2; i++){
    wait(&status);
    if(status != 0){
      fprintf(1, "Error while reading the pipe");
      exit(1);
    }
    bytesRead += (MB/(CHILDREN * 2));
  }


  int elapsed = uptime() - start;

  fprintf(1, "Received %d bytes of data\n", bytesRead);

  fprintf(1, "elapsed time is %d\n", elapsed);

  exit(0);

}
