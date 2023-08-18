#include "date.h"
#include "dividenddownload.h"
#include "genlib.h"
#include "investment.h"
#include "stockcounter.h"
#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// see transactioncounter.h for documentation

// ALL FUNCTIONS REQUIRE VALID POINTERS

// transaction types
static const int BUY = 1;
static const int SELL = 2;
static const int DIV = 3;

static const char *S_BUY = "Buy";
static const char *S_SELL = "Sell";
static const char *S_DIV = "Dividend";

static const char *transaction_fname = "transactioncounter.csv";
static const char *tempname = "transactiontemp.txt";

// prints the line of the transaction
// if type is DIV, n is the number of shares owned, and p is the dividend per
// share if type is not DIV, ex_d can be NULL requires: fp is open to write
// 		ticker is a null-terminated string (not asserted)
// 		n, p > 0
static void transaction_print(FILE *fp, const char *ticker, int n, double p,
                              struct date *d, int type, struct date *ex_d) {
  assert(fp);
  assert(ticker);
  assert(d);
  assert(p > 0);
  assert(n > 0);
  char *date = malloc(DATE_LENGTH * sizeof(char));
  date_string(d, date);
  fprintf(fp, "%s,%s ,", date, ticker);
  free(date);
  if (type == BUY)
    fprintf(fp, S_BUY);
  else if (type == SELL) {
    fprintf(fp, S_SELL);
    n = -n;
  } else
    fprintf(fp, S_DIV);
  fputc(',', fp);
  if (ex_d) {
    char *ex_date = malloc(DATE_LENGTH * sizeof(char));
    date_string(ex_d, ex_date);
    fprintf(fp, ex_date);
    free(ex_date);
  }
  fputc(',', fp);
  fprintf(fp, "%lf,%d,%lf\n", p, n, p * n);
}

// reads a line of fp and assigns the column valules to the pointers
// then returns the fp pointer to where it started
// ALL values can be NULL except for fp
// requires: all date pointers must be either NULL or assigned to a date
static void transaction_read(FILE *fp, struct date **d, char *ticker, int *type,
                             struct date **ex_d, double *price, int *shares,
                             double *total_amount) {
  assert(fp);
  fpos_t position;
  fgetpos(fp, &position);
  char c;
  if (d) {
    char date_s[DATE_LENGTH];
    fgets(date_s, DATE_LENGTH, fp);
    if (*d) {
      date_destroy(*d);
      *d = NULL;
    }
    *d = string_to_date(date_s);
  }
  while (fgetc(fp) != ',')
    ;
  if (ticker) {
    fscanf(fp, "%s", ticker);
  }
  while (fgetc(fp) != ',')
    ;
  if (type) {
    c = fgetc(fp);
    if (c == S_BUY[0])
      *type = BUY;
    else if (c == S_SELL[0])
      *type = SELL;
    else
      *type = DIV;
  }
  while (fgetc(fp) != ',')
    ;
  if (ex_d) {
    // we still want to check if there is an ex_d here, because sometimes we
    // will need to save the date without knowing if the transaction is a
    // dividend or not
    fpos_t pos2;
    fgetpos(fp, &pos2);
    c = fgetc(fp);
    fsetpos(fp, &pos2);
    if (c != ',') {
      char ex_date_s[DATE_LENGTH];
      fgets(ex_date_s, DATE_LENGTH, fp);
      if (*ex_d) {
        date_destroy(*ex_d);
        *ex_d = NULL;
      }
      *ex_d = string_to_date(ex_date_s);
    } else {
      if (*ex_d) {
        date_destroy(*ex_d);
        *ex_d = NULL;
      }
    }
  }
  while (fgetc(fp) != ',')
    ;
  if (price)
    fscanf(fp, "%lf", price);
  while (fgetc(fp) != ',')
    ;
  if (shares)
    fscanf(fp, "%d", shares);
  while (fgetc(fp) != ',')
    ;
  if (total_amount)
    fscanf(fp, "%lf", total_amount);
  fsetpos(fp, &position);
}

// copies the header lines of r, the csv file, into w
// if w is NULL it'll just skip through r
// requires: r is open to read
static void copy_header(FILE *r, FILE *w) {
  assert(r);
  const int NUM_HEADER_LINES = 2;
  char c;
  int count = 0;
  while (count < NUM_HEADER_LINES) {
    c = fgetc(r);
    if (w)
      fputc(c, w);
    if (c == '\n')
      count++;
  }
}

// replaces the top line of the csv with a new CAGR and total value
// requires: cagr, tot > 0
static void update_header(const char *cur_fname, double cagr, double tot) {
  assert(tot > 0);
  assert(cagr > 0);
  FILE *fp = fopen(cur_fname, "r");
  FILE *temp = fopen(tempname, "w");
  while (fgetc(fp) != '\n')
    ;
  cagr -= 1;
  cagr *= 100;
  fprintf(temp, "CAGR: %lf%%, Total Portfolio Value: $%lf\n", cagr, tot);
  char c;
  while (!next_eof(fp))
    fputc(fgetc(fp), temp);
  fclose(fp);
  fclose(temp);
  remove(cur_fname);
  rename(tempname, cur_fname);
}

// builds the counter csv file from scratch from the existing transactions csv
// file
static void build_counter(bool update_price) {
  counter_reset();

  FILE *fp = fopen(transaction_fname, "r");
  if (!fp)
    return;
  copy_header(fp, NULL);
  int num_shares;
  char ticker[MAX_TICKER_LENGTH];
  int type;
  while (!next_eof(fp)) {
    transaction_read(fp, NULL, (char *)ticker, &type, NULL, NULL, &num_shares,
                     NULL);
    if (type != DIV)
      counter_change_shares((const char *)ticker, abs(num_shares),
                            num_shares > 0, update_price);
    while (fgetc(fp) != '\n')
      ;
  }
  fclose(fp);
}

int transaction_execute(const char *ticker, int n, double p, struct date *d,
                        bool buy) {
  assert(n > 0);
  assert(p > 0);
  assert(ticker);
  assert(d);

  int type;
  if (buy)
    type = BUY;
  else
    type = SELL;

  // checks counter to see if you can sell
  build_counter(false);
  int ret = counter_num_shares(ticker);
  if (ret == TICKER_NOT_FOUND && !buy)
    return TICKER_NOT_FOUND;
  if (n > ret && !buy)
    return NOT_ENOUGH_SHARES;

  FILE *fp;
  char ticker_transaction_fname[MAX_TICKER_LENGTH + 4];
  const char *cur_fname;
  strcpy(ticker_transaction_fname, ticker);
  strcat(ticker_transaction_fname, ".csv");
  for (int i = 0; i < 2; i++) {
    if (i == 0)
      cur_fname = transaction_fname;
    else
      cur_fname = ticker_transaction_fname;
    fp = fopen(cur_fname, "r");

    // if there's no file then we create one with the header
    if (!fp) {
      fp = fopen(cur_fname, "w");
      fprintf(fp, "CAGR:,Total Portfolio "
                  "Value:\nDate,Ticker,Transaction_Type,Ex_Dividend_Date,Price/"
                  "Amount,Shares,Total_Amount\n");
      transaction_print(fp, ticker, n, p, d, type, NULL);
      fclose(fp);
      build_counter(false);
      if (i == 0)
        continue;
      return SUCCESS;
    }
    FILE *temp = fopen(tempname, "w");
    copy_header(fp, temp);

    // keep going until all the dates earlier than this transaction are passed,
    // so the csv is in chronological order
    int found_date = 0;
    int num_shares = 0;
    int cur_num_shares = 0;
    struct date *cur_d = NULL;
    char cur_tick[MAX_TICKER_LENGTH];
    int cur_type;
    char c;
    while (!next_eof(fp)) {
      transaction_read(fp, &cur_d, (char *)cur_tick, &cur_type, NULL, NULL,
                       &cur_num_shares, NULL);
      if (!found_date && date_compare(cur_d, d) > 0) {
        if (!buy && n > num_shares) {
          fclose(fp);
          fclose(temp);
          build_counter(false);
          return NOT_ENOUGH_SHARES;
        }
        found_date = 1;
        transaction_print(temp, ticker, n, p, d, type, NULL);
      }
      // if the tickers match, we have to update num_shares
      if (strcmp(ticker, (const char *)cur_tick) == 0)
        num_shares += cur_num_shares;
      c = fgetc(fp);
      while (c != '\n') {
        fputc(c, temp);
        c = fgetc(fp);
      }
      fputc('\n', temp);
    }
    if (!found_date) {
      if (!buy && n > num_shares) {
        fclose(fp);
        fclose(temp);
        build_counter(false);
        return NOT_ENOUGH_SHARES;
      }
      transaction_print(temp, ticker, n, p, d, type, NULL);
    }
    date_destroy(cur_d);
    fclose(fp);
    fclose(temp);
    remove(cur_fname);
    rename(tempname, cur_fname);
    build_counter(false);
  }
  return SUCCESS;
}

// adds a dividend line for ticker on date d
// requires: div > 0
// 	ticker is a null-terminated string (not asserted)
static void transaction_dividend(const char *ticker, double div, struct date *d,
                                 struct date *ex_d) {
  assert(ticker);
  assert(d);
  assert(ex_d);
  assert(div > 0);
  char ticker_transaction_fname[MAX_TICKER_LENGTH + 4];
  const char *cur_fname;
  strcpy(ticker_transaction_fname, ticker);
  strcat(ticker_transaction_fname, ".csv");
  for (int i = 0; i < 2; i++) {
    if (i == 0)
      cur_fname = transaction_fname;
    else
      cur_fname = ticker_transaction_fname;
    FILE *fp = fopen(cur_fname, "r");
    if (!fp)
      return;

    FILE *temp = fopen(tempname, "w");
    copy_header(fp, temp);

    // keep going until all the dates earlier than this transaction are passed,
    // and count how many stocks of ticker are owned
    int found_date = 0;
    int num_shares = 0;
    int cur_num_shares = 0;
    char c;
    struct date *cur_d = NULL;
    char cur_tick[MAX_TICKER_LENGTH];
    int cur_type;
    while (!next_eof(fp)) {
      transaction_read(fp, &cur_d, (char *)cur_tick, &cur_type, NULL, NULL,
                       &cur_num_shares, NULL);
      if (!found_date && date_compare(cur_d, d) > 0) {
        found_date = 1;
        if (num_shares > 0)
          transaction_print(temp, ticker, num_shares, div, d, DIV, ex_d);
      } else if (cur_type == DIV && date_compare(cur_d, d) == 0) {
        fclose(fp);
        fclose(temp);
        remove(tempname);
        return;
      }
      // if the tickers match, we have to update num_shares
      if (strcmp(ticker, (const char *)cur_tick) == 0)
        num_shares += cur_num_shares;
      c = fgetc(fp);
      while (c != '\n') {
        fputc(c, temp);
        c = fgetc(fp);
      }
      fputc('\n', temp);
    }
    date_destroy(cur_d);
    if (!found_date && next_eof(fp) && num_shares > 0)
      transaction_print(temp, ticker, num_shares, div, d, DIV, ex_d);
    fclose(fp);
    fclose(temp);
    remove(cur_fname);
    rename(tempname, cur_fname);
  }
}

int transaction_stock_split(const char *ticker, int n, bool reverse) {
  assert(ticker);
  assert(n >= 1);

  FILE *fp = fopen(transaction_fname, "r");
  FILE *temp = fopen(tempname, "w");
  copy_header(fp, temp);

  char c;
  int shares;
  double price;
  bool found = 0;
  struct date *d = NULL;
  int type;
  char cur_tick[MAX_TICKER_LENGTH];
  while (!next_eof(fp)) {
    transaction_read(fp, &d, (char *)cur_tick, &type, NULL, &price, &shares,
                     NULL);
    if (strcmp((const char *)cur_tick, ticker) == 0 && type != DIV) {
      found = 1;
      if (reverse) {
        shares /= n;
        price *= n;
      } else {
        shares *= n;
        price /= n;
      }
      transaction_print(temp, ticker, abs(shares), price, d, type, NULL);
      while (fgetc(fp) != '\n')
        ;
    } else {
      c = fgetc(fp);
      while (c != '\n') {
        fputc(c, temp);
        c = fgetc(fp);
      }
      fputc('\n', temp);
    }
  }
  date_destroy(d);
  fclose(fp);
  fclose(temp);
  remove(transaction_fname);
  rename(tempname, transaction_fname);
  build_counter(false);
  if (!found)
    return TICKER_NOT_FOUND;
  return SUCCESS;
}

// calculates the cagr, adds it to the csv, and also returns the cagr,
// i.e. 1.15 for 15%
double calculate_cagr() {
  double cagr = 1;

  // adds the dividends to the transaction csv
  build_counter(true);
  struct tickerlist *tl = tickerlist_create();
  int len = tickerlist_size(tl);
  struct date *ex_d = NULL;
  struct date *d = NULL;
  double div;
  const char *ticker;
  FILE *div_fp;
  for (int i = 0; i < len; i++) {
    ticker = tickerlist_nth(tl, i);
    dividend_history_download(ticker);
    div_fp = dividend_open();
    while (!next_eof(div_fp)) {
      dividend_read_line(div_fp, &d, &ex_d, &div);
      transaction_dividend(ticker, div, d, ex_d);
    }
    fclose(div_fp);
    dividend_delete();
  }

  char ticker_transaction_fname[MAX_TICKER_LENGTH + 4];
  const char *cur_fname;
  const char *cur_ticker;
  FILE *fp;
  for (int j = 0; j <= len; j++) {
    if (j == len)
      cur_fname = transaction_fname;
    else {
      cur_ticker = tickerlist_nth(tl, j);
      strcpy(ticker_transaction_fname, cur_ticker);
      strcat(ticker_transaction_fname, ".csv");
      cur_fname = ticker_transaction_fname;
    }
    // adds everything from the csv to the investment
    fp = fopen(cur_fname, "r");
    // if there's no file we return a default cagr
    if (!fp) {
      return cagr;
    }
    copy_header(fp, NULL);

    struct investment *i = investment_create();
    double amount;
    int type;
    struct date *cur_d = NULL;
    while (!next_eof(fp)) {
      transaction_read(fp, &cur_d, NULL, &type, NULL, NULL, NULL, &amount);
      if (type == BUY)
        investment_buy(amount, i, cur_d);
      if (type == SELL)
        investment_sell(amount * -1, i, cur_d);
      if (type == DIV)
        investment_add_dividend(amount, i);

      while (fgetc(fp) != '\n')
        ;
    }
    date_destroy(cur_d);
    fclose(fp);
    if (j == len)
      investment_final_value(counter_total_value(), i);
    else
      investment_final_value(counter_ticker_value(cur_ticker), i);
    cagr = investment_cagr(i);
    double tot = investment_get_total(i);
    investment_destroy(i);
    update_header(cur_fname, cagr, tot);
  }
  tickerlist_destroy(tl);
  return cagr;
}
