#include "dividenddownload.h"
#include "date.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

const char *dividend_fname = "dividends.csv";
static const char *tempname = "temp.csv";

void dividend_history_download(const char *ticker) {
  assert(ticker);

  // builds url and downloads it
  remove(dividend_fname);
  char command[150] = "curl -o ";
  strcat(command, dividend_fname);
  strcat(command, " https://dividendhistory.org/payout/");
  strcat(command, ticker);
  strcat(command, "/");
  system(command);

  // extracts dividend data from file

  // first pass through: deletes data preceding dividend stuff, and deletes tags
  // and most lines that don't have dividend data
  FILE *temp = fopen(tempname, "w");
  FILE *fp = fopen(dividend_fname, "r");
  char *key = "<table";
  char cur[1000];
  int found = 0;
  while (!found) {
    fscanf(fp, "%s", cur);
    if (strcmp((const char *)key, (const char *)cur) == 0) {
      found = 1;
    } else {
      while (fgetc(fp) != '\n')
        ;
    }
  }
  int in = 1;
  char c;
  while (!feof(fp)) {
    c = fgetc(fp);
    if (in) {
      if (c == '>')
        in = 0;
    } else {
      if (c == '<')
        in = 1;
      // accepted character list is 0-9 . % $ - because those are the only
      // characters in actual dividend data, and \n to make the lines separate
      else if ((c >= '0' && c <= '9') || c == '.' || c == '%' || c == '$' ||
               c == '-' || c == '\n')
        fputc(c, temp);
    }
  }
  fclose(fp);
  fclose(temp);
  remove(dividend_fname);
  rename(tempname, dividend_fname);

  // second pass through: deletes all lines that aren't dividend data, turns it
  // into a csv, and deletes dividend data for future payments
  fp = fopen(dividend_fname, "r");
  temp = fopen(tempname, "w");
  char line[100];
  struct date *d;
  while (!feof(fp)) {
    fscanf(fp, "%s", line);
    if ((line[0] == '2' || line[0] == '1') &&
        strchr((const char *)line, '%') == NULL &&
        strlen((const char *)line) == 10) {
      d = string_to_date((const char *)line);
      if (future(d)) {
        fclose(temp);
        temp = fopen(tempname, "w");
      } else
        fprintf(temp, "%s,", line);
      date_destroy(d);
    } else if (line[0] == '$')
      fprintf(temp, "%s\n", line + 1);
  }
  fclose(fp);
  fclose(temp);
  remove(dividend_fname);
  rename(tempname, dividend_fname);

  // third pass: adds title line, removes leftover text from future dividends
  fp = fopen(dividend_fname, "r");
  temp = fopen(tempname, "w");
  fprintf(temp,
          "%s dividends\nEx Dividend Date,Dividend Payout Date,Amount($)\n",
          ticker);
  while (!feof(fp)) {
    fgets(line, 100, fp);
    if (strlen(line) >= DATE_LENGTH * 2)
      fputs(line, temp);
  }
  fclose(fp);
  fclose(temp);
  remove(dividend_fname);
  rename(tempname, dividend_fname);
}

// opens the dividend file to read, and skips the opening lines
// returns a pointer to the dividend file
// effects: opens a file pointer, user must fclose() the return value
FILE *dividend_open() {
  FILE *fp = fopen(dividend_fname, "r");
  // skip two lines
  while (fgetc(fp) != '\n')
    ;
  while (fgetc(fp) != '\n')
    ;
  return fp;
}

// reads a line of the dividend csv and assigns the column values to the
// pointers
void dividend_read_line(FILE *fp, struct date **d, struct date **ex_d,
                        double *amount) {
  assert(fp);
  assert(d);
  assert(ex_d);
  assert(amount);
  char date_s[DATE_LENGTH];
  char ex_date_s[DATE_LENGTH];
  fgets(ex_date_s, DATE_LENGTH, fp);
  while (fgetc(fp) != ',')
    ;
  fgets(date_s, DATE_LENGTH, fp);
  while (fgetc(fp) != ',')
    ;
  fscanf(fp, "%lf", amount);
  while (fgetc(fp) != '\n')
    ;
  if (*ex_d)
    date_destroy(*ex_d);
  *ex_d = string_to_date(ex_date_s);
  if (*d)
    date_destroy(*d);
  *d = string_to_date(date_s);
}

void dividend_delete() { remove(dividend_fname); }
