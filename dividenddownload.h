#ifndef DIVIDENDDOWNLOAD_H
#define DIVIDENDDOWNLOAD_H
#include <stdbool.h>
#include <stdio.h>
#include "date.h"
// a few functions to downlaod dividend history data off of nasdaq.com
// Made by Marco Calabretta under request of my dad

// All functions require valid pointers

// the dividends are downloaded into this file
extern const char *dividend_fname;

// downloads the dividend history into a csv into the current folder
// requires: ticker is a null-terminated string (not asserted)
// 		ticker is a valid stock ticker (not asserted)
// effects: creates a file, user should call dividend_delete
void dividend_history_download(const char *ticker);

// opens the dividend file to read, and skips the opening lines
// returns a pointer to the dividend file
// effects: opens a file pointer, user must fclose() the return value
FILE *dividend_open();

// reads a line of the dividend csv and assigns the column values to the
// pointers
void dividend_read_line(FILE *fp, struct date **d, struct date **ex_d,
                        double *amount);

// deletes the dividend file
void dividend_delete();
#endif
