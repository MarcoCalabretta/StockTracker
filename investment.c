#include "investment.h"
#include "date.h"
#include "polynomial.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

// allowable margin of error, 0.01 means 1 cent off
double ERROR = 0.001;

// see calc.h for documentation

struct investment {
  struct date *end_date;
  double end_money;
  double dividends;
  struct polynomial *trades;
};

struct investment *investment_create() {
  struct investment *i = malloc(sizeof(struct investment));
  i->end_date = current_date();
  i->end_money = 0;
  i->dividends = 0;
  i->trades = polynomial_create();
  return i;
}

void investment_destroy(struct investment *i) {
  assert(i);
  date_destroy(i->end_date);
  polynomial_destroy(i->trades);
  free(i);
}

void investment_buy(double d, struct investment *i, struct date *buy_date) {
  assert(i);
  assert(buy_date);
  assert(d > 0);
  if (d > i->dividends) {
    d -= i->dividends;
    i->dividends = 0;
    int days_held = date_compare(i->end_date, buy_date);
    double e = (double)days_held * YEARS_PER_DAY;
    polynomial_add_term(d, e, i->trades);
  } else {
    i->dividends -= d;
  }
}

void investment_sell(double d, struct investment *i, struct date *sell_date) {
  assert(i);
  assert(sell_date);
  assert(d > 0);
  int days_since_sold = date_compare(i->end_date, sell_date);
  double e = days_since_sold * YEARS_PER_DAY;
  polynomial_add_term(d * -1, e, i->trades);
}

void investment_add_dividend(double d, struct investment *i) {
  assert(i);
  assert(d > 0);
  i->dividends += d;
}

// gets the total value of the investment i, including dividends
double investment_get_total(struct investment *i) {
  assert(i);
  return i->dividends + i->end_money;
}

void investment_final_value(double d, struct investment *i) {
  assert(i);
  assert(d > 0);
  i->end_money = d;
}

double investment_cagr(struct investment *i) {
  assert(i);
  polynomial_add_term((i->end_money + i->dividends) * -1, 0, i->trades);
  return newtons_method(ERROR, 1, i->trades);
}
