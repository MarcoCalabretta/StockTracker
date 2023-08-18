#include "stockcounter.h"
#include "genlib.h"
#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// see stockcounter.h for documentation

static const char *counter_first_line = "Ticker,Number of Shares,Price\n";
const char *counter_fname = "stockcounter.csv";
static const char *tempname = "tempcounter.csv";
static const char *temppricename = "tempprice.txt";

// copies the header lines of r, the csv file, into w
// if w is NULL it'll just skip through r
// requires: r is open to read
static void copy_first_line(FILE *r, FILE *w) {
  assert(r);
  char c;
  do {
    c = fgetc(r);
    if (w)
      fputc(c, w);
  } while (c != '\n');
}

// returns the current price of ticker
// effects: Nothing! pog
static double get_price(const char *ticker) {
  assert(ticker);

  // builds url and downloads it
  remove(temppricename);
  char command[150] = "curl -o ";
  strcat(command, temppricename);
  strcat(command, " https://www.marketwatch.com/investing/stock/");
  strcat(command, ticker);
  system(command);

  FILE *fp = fopen(temppricename, "r");
  bool file_ok = true;
  if (!fp)
    file_ok = false;
  else {
    fgetc(fp);
    if (feof(fp))
      file_ok = false;
  }
  // if there's no file, we try building the url again with "fund" instead of
  // "stock"
  if (!file_ok) {
    fclose(fp);
    remove(temppricename);
    strcpy(command, "curl -o ");
    strcat(command, temppricename);
    strcat(command, " https://www.marketwatch.com/investing/fund/");
    strcat(command, ticker);
    system(command);

    fp = fopen(temppricename, "r");
    file_ok = true;
    if (!fp)
      file_ok = false;
    else {
      fgetc(fp);
      if (feof(fp))
        file_ok = false;
    }
  }
  // if it's not a fund or a stock we don't know what it is
  if (!file_ok)
    return 0;

  // searches for the pattern immediately preceding the price
  char line[100];
  const char *key = "        <meta name=\"price\" content=\"$";
  int keylen = strlen(key);
  int linelen = 0;
  double price;
  bool diff = 0;
  bool found = 0;
  while (!found && !feof(fp)) {
    fgets(line, 100, fp);
    diff = 0;
    linelen = strlen((const char *)line);
    for (int i = 0; i < keylen && i < linelen; i++) {
      if (line[i] != key[i]) {
        diff = 1;
        break;
      }
    }
    if (!diff) {
      found = 1;
      price = strtod((const char *)(line + keylen), NULL);
    }
  }

  fclose(fp);
  remove(temppricename);
  return price;
}

// returns true if the counter file exists, false otherwise
static bool counter_exists() {
  bool exists;
  FILE *fp = fopen(counter_fname, "r");
  if (fp)
    exists = true;
  else
    exists = false;
  fclose(fp);
  return exists;
}

void counter_reset() {
  remove(counter_fname);
  FILE *fp = fopen(counter_fname, "w");
  fprintf(fp, counter_first_line);
  fclose(fp);
}

int counter_change_shares(const char *ticker, int n, bool buy,
                          bool update_price) {
  assert(ticker);
  assert(n >= 1);
  if (!buy)
    n = -n;
  if (!counter_exists())
    counter_reset();

  FILE *temp = fopen(tempname, "w");
  FILE *fp = fopen(counter_fname, "r");
  copy_first_line(fp, temp);
  char cur_ticker[MAX_TICKER_LENGTH];
  int shares;
  bool found = false;
  double price = 1;
  double cur_price;
  while (!feof(fp) &&
         fscanf(fp, "%s ,%d,%lf\n", cur_ticker, &shares, &cur_price) == 3) {
    if (!found && strcmp(ticker, cur_ticker) == 0) {
      found = true;
      price = cur_price;
      shares += n;
      if (shares < 0) {
        fclose(fp);
        fclose(temp);
        remove(tempname);
        return NOT_ENOUGH_SHARES;
      }
    }
    if (shares > 0)
      fprintf(temp, "%s ,%d,%lf\n", cur_ticker, shares, cur_price);
  }
  if (!found && buy) {
    if (update_price)
      price = get_price(ticker);
    fprintf(temp, "%s ,%d,%lf\n", ticker, n, price);
  }
  fclose(fp);
  fclose(temp);
  if (found || buy) {
    remove(counter_fname);
    rename(tempname, counter_fname);
    return SUCCESS;
  } else {
    remove(tempname);
    return TICKER_NOT_FOUND;
  }
}

int counter_num_shares(const char *ticker) {
  assert(ticker);
  if (!counter_exists())
    counter_reset();
  FILE *fp = fopen(counter_fname, "r");
  copy_first_line(fp, NULL);

  // find ticker, return number associated with ticker
  char cur_ticker[MAX_TICKER_LENGTH];
  int shares;
  double price;
  while (!feof(fp) &&
         fscanf(fp, "%s ,%d,%lf\n", cur_ticker, &shares, &price) == 3) {
    if (strcmp(ticker, cur_ticker) == 0) {
      fclose(fp);
      return shares;
    }
  }
  fclose(fp);
  return TICKER_NOT_FOUND;
}

// returns the total value of all the shares of ticker
// requires: ticker is a null-terminated string (not asserted)
// will return TICKER_NOT_FOUND if ticker is not present
double counter_ticker_value(const char *ticker) {
  assert(ticker);
  if (!counter_exists())
    counter_reset();
  FILE *fp = fopen(counter_fname, "r");
  copy_first_line(fp, NULL);

  // find ticker, return number associated with ticker
  char cur_ticker[MAX_TICKER_LENGTH];
  int shares;
  double price;
  while (!feof(fp) &&
         fscanf(fp, "%s ,%d,%lf\n", cur_ticker, &shares, &price) == 3) {
    if (strcmp(ticker, cur_ticker) == 0) {
      fclose(fp);
      return (double)shares * price;
    }
  }
  fclose(fp);
  return (double)TICKER_NOT_FOUND;
}

double counter_total_value() {
  if (!counter_exists())
    counter_reset();
  double tot_val = 0;
  FILE *fp = fopen(counter_fname, "r");
  copy_first_line(fp, NULL);

  // iterate through the file, adding all the total values of the individual
  // stocks
  char cur_ticker[MAX_TICKER_LENGTH];
  int shares;
  double price;
  while (!feof(fp)) {
    fscanf(fp, "%s ,%d,%lf\n", cur_ticker, &shares, &price);
    tot_val += shares * price;
  }
  fclose(fp);
  return tot_val;
}

struct tickerlist {
  int length;
  char **tickers;
};

struct tickerlist *tickerlist_create() {
  if (!counter_exists())
    counter_reset();
  struct tickerlist *t = malloc(sizeof(struct tickerlist));
  t->length = 0;
  int max_length = 1;
  t->tickers = malloc(max_length * sizeof(char *));
  char **temp;
  FILE *fp = fopen(counter_fname, "r");
  while (!next_eof(fp)) {
    if (fgetc(fp) == '\n') {
      if (t->length >= max_length) {
        max_length *= 2;
        temp = realloc(t->tickers, max_length * sizeof(char *));
        if (temp)
          t->tickers = temp;
      }
      (t->tickers)[t->length] = malloc(MAX_TICKER_LENGTH * sizeof(char));
      fscanf(fp, "%s", (t->tickers)[t->length]);
      t->length++;
    }
  }
  t->length--;
  fclose(fp);
  return t;
}

int tickerlist_size(struct tickerlist *t) {
  assert(t);
  return t->length;
}

// returns the nth element of t, in array style (starting from 0, ending at len
// - 1) requires: n >= 0, n < tickerlist_size
const char *tickerlist_nth(struct tickerlist *t, int n) {
  assert(t);
  assert(n >= 0);
  assert(n < tickerlist_size(t));
  return (t->tickers)[n];
}

void tickerlist_destroy(struct tickerlist *t) {
  assert(t);
  for (int i = 0; i <= t->length; i++) {
    free((t->tickers)[i]);
  }
  free(t->tickers);
  free(t);
}
