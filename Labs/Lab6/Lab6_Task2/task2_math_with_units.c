/*
* task2_math_with_units.c
* 2025F COMP-166 Lab 6 - Thomas Boland - C0556991
*
* This program performs basic arithmetic operations on measurements with units.
*
*/

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include "units_math.h"

int main() {
	SiValue valA, valB, result;
	char operator;
	while (true) {
		// read two measurements separated by an operator (+, -, *, /) from stdin
		if (siValueParseFromStream(stdin, &valA) != 0) {
			fprintf(stderr, "Error: Invalid input for first measurement.\n");
			int c;
			while ((c = getchar()) != '\n' && c != EOF) {}
			continue;
		}
		// read operator
		// skip whitespace, read one [+-*/]
		if (fscanf_s(stdin, " %[-+\*/]", &operator, 1) != 1) {
			fprintf(stderr, "Error: Invalid or missing operator.\n");
			int c;
			while ((c = getchar()) != '\n' && c != EOF) {}
			continue;
		}
		if (siValueParseFromStream(stdin, &valB) != 0) {
			fprintf(stderr, "Error: Invalid input for second measurement.\n");
			int c;
			while ((c = getchar()) != '\n' && c != EOF) {}
			continue;
		}

		// if adding or subtracting, check that units are the same
		if ((operator == '+' || operator == '-' ) &&!siValueSameUnit(valA, valB)) {
			fprintf(stderr, "Error: Units must be the same when adding or subtracting.\n");
			int c;
			while ((c = getchar()) != '\n' && c != EOF) {}
			continue;
		}
	}
	
	// perform the operation
	switch (operator) {
	case '+': // TODO addition and subtraction fail if different units.
		if (siValueAdd(valA, valB, &result) != 0) {
			fprintf(stderr, "Error: Units must be the same when adding or subtracting.\n");
		}
		break;
	case '-': // TODO addition and subtraction fail if different units.
		if (siValueSubtract(valA, valB, &result) != 0) {
			fprintf(stderr, "Error: Units must be the same when adding or subtracting.\n");
		}
		break;
	case '*':
		if (siValueMultiply(valA, valB, &result) != 0) {
			fprinf(stderr, "Error: Multiplication failed.\n");
		}
		break;
	case '/':
		if (siValueDivide(valA, valB, &result) != 0) {
			fprintf(stderr, "Error: Division by zero.\n");
		}
		break;
	default:
		fprintf(stderr, "Error: Unknown operator '%c'.\n", operator);
		return EXIT_FAILURE;
	}

	// output result
	siValueOutputToStream(stdout, &result);
	return EXIT_SUCCESS;
}