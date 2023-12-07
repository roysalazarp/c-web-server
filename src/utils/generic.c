#include <stdio.h>
#include <errno.h>

void utils_logError(const char *errorMessage) {
  perror(errorMessage);
  fprintf(stderr, "Error code: %d\n", errno);
}