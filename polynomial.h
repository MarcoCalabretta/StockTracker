#ifndef POLYNOMIAL_H
#define POLYNOMIAL_H
// this defines a polynomial struct and has some methods to find roots of a
// polynomial Made by Marco Calabretta

// all functions require valid pointers

// a polynomial with real coefficients and real (i.e. potentiall non-integer)
// exponents
struct polynomial;

// makes a new empty polynomial
// effects: allocates heap memory, client must call polynomial_destroy
// time: O(1)
struct polynomial *polynomial_create();

// frees all memory associated with p
// time: O(1)
void polynomial_destroy(struct polynomial *p);

// returns the number of terms in p
int polynomial_num_terms(struct polynomial *p);

// adds a new term (a*x^e) to the polynomial p
// time: O(1) (amortized)
void polynomial_add_term(double a, double e, struct polynomial *p);

// evaluates p at x
// requires: x >= 0 (this is because the exponents are not all integers)
// time: O(n), where n is the number of terms in p
double polynomial_evaluate(double x, struct polynomial *p);

// returns an approximation to a root of p within error err, using newton's
// method, starting with x0 as the first guess requires: err > 0
// time: O(1) (I put an artificial halter in because newton's method is not
// guaranteed to stop
double newtons_method(double err, double x0, struct polynomial *p);
#endif
