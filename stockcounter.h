#ifndef STOCKCOUNTER_H
#define STOCKCOUNTER_H
// this file has a bunch of functions to deal with the stock counter csv file
// Made by Marco Calabretta

#include <stdbool.h>

// All functions require valid pointers

// the filename for the csv file
extern const char *counter_fname;

// resets the counter file to include just the header
void counter_reset();

// either adds or removes n shares of ticker
// If you buy a stock that doesn't exist, it will be added to the csv
// returns SUCCESS if it worked, TICKER_NOT_FOUND if you tried to sell a ticker
// that doesn't exist, and NOT_ENOUGH_SHARES if you didn't have enough shares to
// sell requires: ticker is a null-terminated string (not asserted) n
// >= 1
int counter_change_shares(const char *ticker, int n, bool buy,
                          bool update_prices);

// returns the number of owned shares of ticker
// requires: ticker is a null-terminated string (not asserted)
// will return TICKER_NOT_FOUND if ticker is not present
int counter_num_shares(const char *ticker);

// returns the total value of all the shares of ticker
// requires: ticker is a null-terminated string (not asserted)
// will return TICKER_NOT_FOUND if ticker is not present
double counter_ticker_value(const char *ticker);

// returns the total value of all the shares in the csv file
double counter_total_value();

// a list of all the tickers in the file
struct tickerlist;

// returns a pointer to a tickerlist
// effects: allocates heap memory, user must call tickerlist_destroy
struct tickerlist *tickerlist_create();

// returns the length of t
int tickerlist_size(struct tickerlist *t);

// returns the nth element of t, in array style (starting from 0, ending at len
// - 1) requires: n >= 0, n < tickerlist_size
const char *tickerlist_nth(struct tickerlist *t, int n);

// destroys all memory associated with t, rendering it null
void tickerlist_destroy(struct tickerlist *t);
#endif
