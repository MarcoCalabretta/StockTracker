#include "date.h"
#include "genlib.h"
#include "transactioncounter.h"
#include <stdio.h>

const int BUY = 1;
const int SELL = 2;
const int CAGR = 3;
const int SPLIT = 4;

int main() {
  int dec = 0;
  int ret;
  char ticker[MAX_TICKER_LENGTH];

  while (dec < BUY || dec > SPLIT) {
    printf("Would you like to buy (1), sell (2), calculate CAGR (3), or notify "
           "about a stock split (4)? ");
    scanf("%d", &dec);
    if (dec < BUY || dec > SPLIT)
      printf("ERROR. Must input a number from 1 to 4\n");
  }

  if (dec == SPLIT) {
    int split = 0;
    printf("Insert ticker: ");
    scanf("%s", ticker);
    while (split < 1 || split > 2) {
      printf("Is it a split (1) or a reverse split (2)? ");
      scanf("%d", &split);
      if (split < 1 || split > 2)
        printf("ERROR. Must input a number from 1 to 2\n");
    }

    if (split == 1) {
      split = 0;
      while (split <= 0) {
        printf("How big is the split? e.g. input 3 for a 3-for-1 split: ");
        scanf("%d", &split);
        if (split <= 0)
          printf("ERROR. Must input a number greater than 0\n");
      }
      ret = transaction_stock_split((const char *)ticker, split, false);
    } else {
      split = 0;
      while (split <= 0) {
        printf("How big is the reverse split? e.g. input 3 for a 1-for-3 "
               "reverse split: ");
        scanf("%d", &split);
        if (split <= 0)
          printf("ERROR. Must input a number greater than 0\n");
      }
      ret = transaction_stock_split((const char *)ticker, split, true);
    }
  }

  // cagr calculate
  else if (dec == CAGR) {
    double cagr = calculate_cagr();
    cagr -= 1;
    cagr *= 100;
    printf("The total CAGR for your portfolio is %lf%% annually\n", cagr);
  }

  // transaction, either buy or sell
  else {
    printf("Which stock ticker would you like to transact? ");
    scanf("%s", ticker);
    int num = 0;
    while (num <= 0) {
      printf("How many shares did you transact? ");
      scanf("%d", &num);
      if (num <= 0)
        printf("ERROR. Must input a number greater than 0\n");
    }
    double price = 0;
    while (price <= 0) {
      printf("What price were the shares? ");
      scanf("%lf", &price);
      if (price <= 0)
        printf("ERROR. Must input a number greater than 0\n");
    }
    int month = 0;
    while (month < 1 || month > 12) {
      printf("What month did this transaction occur (eg. 5 for may)? ");
      scanf("%d", &month);
      if (month < 1 || month > 12)
        printf("ERROR. Must input a valid month (1-12)\n");
    }
    int day = 0;
    while (day < 1 || day > 31) {
      printf("What day did this transaction occur? ");
      scanf("%d", &day);
      if (day < 1 || day > 31)
        printf("ERROR. Must input a valid day (1-31)\n");
    }
    int year = 0;
    while (year < 1900 || year > 2100) {
      printf("What year did this transaction occur? ");
      scanf("%d", &year);
      if (year < 1900 || year > 2100)
        printf("ERROR. Must input a valid year (1900-2100)\n");
    }
    int buy;
    if (dec == BUY)
      buy = 1;
    else
      buy = 0;
    struct date *d = date_create(day, month, year);
    ret = transaction_execute((const char *)ticker, num, price, d, buy);
    date_destroy(d);
  }
  if (ret == SUCCESS)
    printf("Success!\n");
  if (ret == TICKER_NOT_FOUND)
    printf("Error: The ticker you entered was not found!\n");
  if (ret == NOT_ENOUGH_SHARES)
    printf("ERROR: You do not have enough shares to sell!\n");
}
