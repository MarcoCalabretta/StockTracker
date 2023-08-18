#ifndef TRANSACTIONCOUNTER_H
#define TRANSACTIONCOUNTER_H
// this file has a bunch of functions to deal with the transaction counter csv
// file Made by Marco Calabretta

// All functions require valid pointers

#include "date.h"
#include <stdbool.h>

// buys or sells n shares of ticker at price p, on date d
// buy is true if the transaction is a buy, false if it is a sell
// returns SUCCESS if it worked, TICKER_NOT_FOUND if you tried to sell a ticker
// that doesn't exist, and NOT_ENOUGH_SHARES if you didn't have enough shares to
// sell requires: n, p > 0 	ticker is a null-terminated string (not
// asserted)
int transaction_execute(const char *ticker, int n, double p, struct date *d,
                        bool buy);

// makes an n for 1 stock split for ticker (or a 1 for n reverse split if
// reverse is true), so for every stock you now get n stocks returns SUCCESS if
// it worked and TICKER_NOT_FOUND if you tried to split a nonexistent ticker
// requires: ticker is a null-terminated string (not asserted) 		n >= 1
int transaction_stock_split(const char *ticker, int n, bool reverse);

// calculates the cagr, adds it to the csv, and also returns the cagr, i.e. 1.15
// for 15%
double calculate_cagr();
#endif
