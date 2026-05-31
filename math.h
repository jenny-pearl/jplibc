#include <stdio.h>
#include <stdint.h>

#ifndef MATH_H_
#define MATH_H_

#define PI 3.14159265358979323846f

#define ABS(x) ((x) > 0 ? (x) : -(x))

int64_t factorial(int64_t number)
{
	int64_t result = 1;

	for (int64_t i = number; i > 1; result *= i, i -= 1)
		;

	return result;
}

double power(double base, int64_t exponent)
{
	double result = 1.0;

	for (int64_t i = 0; i < exponent; i += 1) {
		result *= base;
	}

	return result;
}

double sin(double degree)
{
	double term = degree;
	double result = degree;

	for (int64_t i = 0; i < 10; i += 1) {
		term = -term * degree * degree / (2 * i + 2) / (2 * i + 3);
		result += term;
	}

	return result;
}

double sqrt(double number)
{
	double temp = 0;
	double guess = number / 2;

	double precision = 1e-5;

	do {
		temp = guess;
		guess = guess - ((guess * guess - number) / (2 * guess));
	} while (ABS(guess - temp) > precision);

	return guess;
}

#endif // MATH_H_
