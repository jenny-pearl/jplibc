#include <stdio.h>
#include <stdint.h>
#include <unistd.h>

#define M_PI 3.141592653589793
#define M_E 2.718281828845905

#define ARRAY_LENGTH(x) (sizeof((x)) / sizeof((x)[0]))

// f(x) = x^2 - a
// f'(x) = 2x

double inline absd(double number) {
	return number > 0 ? number : -number;
}

double inline power_int(double base, uint64_t power) {
	double result = 1;
	for (uint64_t i = 0; i < power; i++) {
		result *= base;
	}
	return result;
}

double inline newton_method_root(double number, uint64_t rootDegree) {
	double precision = 1e-15;
	double guess = number;
	double temp = 0;

	if (rootDegree < 1) {
		return 1;
	}

	do {
		temp = guess;
		guess = guess - (power_int(guess, rootDegree) - number) /
			(power_int(guess, rootDegree - 1) * rootDegree);
	} while (absd(guess - temp) > precision);

	return guess;
}

double inline power_double(double base, double power) {
	uint64_t wholePower = power / 1;
	uint64_t decimalPower = 0;

	if (wholePower != power) {
		decimalPower = 1 / (power - wholePower);
	}

	double wholePart = power_int(base, wholePower);

	double decimalPart = newton_method_root(base, decimalPower);

	base = wholePart * decimalPart;

	return base;
}

double inline exp(double power) {
	return power_double(M_E, power);
}

double inline sin(double degree) {
	return degree;
}

int main() {
	printf("%lf\n", exp(2));
}
