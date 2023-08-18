#ifndef INVESTMENT_H
#define INVESTMENT_H
#include "date.h"
// a list of functions to deal with the investment struct
// made by Marco Calabretta at the request of my dad

// all functions require valid pointers as arguments

// an investment vehicle that you would like to calculate the cagr of, i.e. a
// stock or a stock portfolio
struct investment;

// creates an investment vehicle (stock portfolio, real estate portfolio, etc)
// and returns a pointer to it effects: allocates heap memory, client must call
// investment_delete time: O(1)
struct investment *investment_create();

// frees all memory associated with i
// time: O(1)
void investment_destroy(struct investment *i);

// buys d dollars of investment i at date (day, month, year)
// effects: mutates *i through pointers
// requires: d > 0
void investment_buy(double d, struct investment *i, struct date *buy_date);

// sells d dollars of investment i at date (day, month, year)
// effects: mutates *i through pointers
// requires: d > 0
void investment_sell(double d, struct investment *i, struct date *sell_date);

// adds a dividend of d dollars to the investment
// requires: d > 0
void investment_add_dividend(double d, struct investment *i);

// gets the total value of the investment i, including dividends
double investment_get_total(struct investment *i);

// sets the final value of your investment at d dollars, excluding dividends
// requires: d > 0
void investment_final_value(double d, struct investment *i);

// returns the cagr of i as a percentage. e.g. 12.34 means a 12.34% annual
// return
double investment_cagr(struct investment *i);
#endif
