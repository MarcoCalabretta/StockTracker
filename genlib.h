#ifndef GENLIB_H
#define GENLIB_H
// some error codes for functions
// Made by Marco Calabretta

#include <stdbool.h>
#include <stdio.h>

extern const int MAX_TICKER_LENGTH;
extern const int SUCCESS;
extern const int TICKER_NOT_FOUND;
extern const int NOT_ENOUGH_SHARES;

// checks if the end of the file is in a char or is right now without moving fp
bool next_eof(FILE *fp);
#endif
