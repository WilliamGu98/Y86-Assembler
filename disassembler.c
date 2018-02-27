#include <stdio.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <unistd.h>
#include <inttypes.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include "printRoutines.h"

#define ERROR_RETURN -1
#define SUCCESS 0

int main(int argc, char **argv) {

  FILE *machineCode, *outputFile;
  long currAddr = 0;

  // Verify that the command line has an appropriate number
  // of arguments

  if (argc < 3 || argc > 4) {
    printf("Usage: %s InputFilename OutputFilename [startingOffset]\n", argv[0]);
    return ERROR_RETURN;
  }

  // First argument is the file to read, attempt to open it
  // for reading and verify that the open did occur.
  machineCode = fopen(argv[1], "rb");

  if (machineCode == NULL) {
    printf("Failed to open %s: %s\n", argv[1], strerror(errno));
    return ERROR_RETURN;
  }

  // Second argument is the file to write, attempt to open it
  // for writing and verify that the open did occur.
  outputFile = fopen(argv[2], "w");

  if (outputFile == NULL) {
    printf("Failed to open %s: %s\n", argv[2], strerror(errno));
    fclose(machineCode);
    return ERROR_RETURN;
  }

  // If there is a 3rd argument present it is an offset so
  // convert it to a value.
  if (4 == argc) {
    // See man page for strtol() as to why we check for errors by examining errno
    errno = 0;
    currAddr = strtol(argv[3], NULL, 0);
    if (errno != 0) {
      perror("Invalid offset on command line");
      fclose(machineCode);
      fclose(outputFile);
      return ERROR_RETURN;
    }
  }

  printf("Opened %s, starting offset 0x%lX\n", argv[1], currAddr);
  printf("Saving output to %s\n", argv[2]);

  /* Comment or delete the following line and this comment before
   * handing in your final version.
   */
  //samplePrint(outputFile);

  // Your code starts here.
  readMachineCode(outputFile, machineCode, currAddr); //Test that we can read machine code

  fclose(machineCode);
  fclose(outputFile);
  return SUCCESS;
}
