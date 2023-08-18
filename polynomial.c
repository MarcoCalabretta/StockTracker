#include "polynomial.h"
#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

// all documentation is in polynomial.h

struct polynomial {
  int len;
  int max_len;
  double (*terms)[2];
};

struct polynomial *polynomial_create() {
  struct polynomial *p = malloc(sizeof(struct polynomial));
  p->len = 0;
  p->max_len = 1;
  p->terms = malloc(p->max_len * sizeof(double[2]));
  return p;
}

void polynomial_destroy(struct polynomial *p) {
  assert(p);
  free(p->terms);
  free(p);
}

int polynomial_num_terms(struct polynomial *p) {
  assert(p);
  return p->len;
}

void polynomial_add_term(double a, double e, struct polynomial *p) {
  assert(p);
  if (p->len >= p->max_len) {
    p->max_len *= 2;
    double(*temp)[2];
    temp = realloc(p->terms, p->max_len * sizeof(double[2]));
    if (temp)
      p->terms = temp;
  }
  (p->terms)[p->len][0] = a;
  (p->terms)[p->len][1] = e;
  p->len++;
}

double polynomial_evaluate(double x, struct polynomial *p) {
  assert(p);
  assert(x >= 0);
  double sum = 0;
  double cur = 0;
  for (int i = 0; i < p->len; i++) {
    cur = (p->terms)[i][0] * pow(x, (p->terms)[i][1]);
    sum += cur;
  }
  return sum;
}

// returns a pointer to the derivative of p
// requires: p is a valid pointer
// effects: allocates heap memory, client must call polynomial_destroy on the
// return value of this function
// time: O(n), where n is the number of terms in p
static struct polynomial *polynomial_derive(struct polynomial *p) {
  assert(p);
  struct polynomial *p_prime = polynomial_create();
  double a = 0;
  double e = 0;
  for (int i = 0; i < p->len; i++) {
    a = (p->terms)[i][0];
    e = (p->terms)[i][1];
    polynomial_add_term(a * e, e - 1, p_prime);
  }
  return p_prime;
}

double newtons_method(double err, double x0, struct polynomial *p) {
  assert(p);
  assert(err > 0);
  double x = x0;
  double x_next;
  double val = polynomial_evaluate(x, p);
  struct polynomial *p_prime = polynomial_derive(p);

  // artificial thing to halt in case program doesn't halt after 10000
  // iterations
  int count = 0;
  while (fabs(val) >= err && count < 10000) {
    x_next = x - val / polynomial_evaluate(x, p_prime);
    if (x_next < 0)
      x_next = x / 2;
    x = x_next;
    val = polynomial_evaluate(x, p);
    count++;
  }
  polynomial_destroy(p_prime);
  return x;
}

// test case, uncomment if needed
/*
int main() {
  struct polynomial *p = polynomial_create();
  polynomial_add_term(-0.023, 4, p);
  polynomial_add_term(0.453, 3, p);
  polynomial_add_term(-2.616, 2, p);
  polynomial_add_term(4.125, 1, p);
  polynomial_add_term(2.36018, 0, p);
  double r1 = newtons_method(0.000001, 4.125, p);
  double r2 = newtons_method(0.00001, 6, p);
  printf("%f and %f", r1, r2);
  polynomial_destroy(p);
}
*/
