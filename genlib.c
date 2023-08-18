// see genlib.h for documentation
#include "genlib.h"
#include <assert.h>
#include <stdbool.h>
#include <stdio.h>

const int MAX_TICKER_LENGTH = 12;
const int SUCCESS = 0;
const int TICKER_NOT_FOUND = -1;
const int NOT_ENOUGH_SHARES = 1;

bool next_eof(FILE *fp) {
  assert(fp);
  if (feof(fp))
    return true;
  fpos_t position;
  fgetpos(fp, &position);
  fgetc(fp);
  if (feof(fp))
    return true;
  fsetpos(fp, &position);
  return false;
}
