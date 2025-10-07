/*
* 2025F COMP-166 Lab 3 - Thomas Boland - C0556991
*
* This program prints a right triangle, isosceles triangle, or diamond (inverted or not), using a specified character.
*
*/

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <errno.h>

/**
* @brief Get an integer input from stdin. Force the user to retry until input is valid.
* @param *input_value Where to put successful input
* @param *prompt String to display for each prompt
* @return Returns 0 if successful, non-zero if failed
*/
int getIntInput(int* input_value, char* prompt) {
	char input_buf[128];
	long input;
	while (true) {
		printf(prompt);

		// get up to 128 characters of input or up to newline
		// fgets returns NULL in the event reading input failed
		if (fgets(input_buf, 128, stdin) == NULL) {
			// Failed to read input
			return -1;
		}

		char* first_unconverted_character;
		errno = 0; // I'm not a fan of the errno pattern, but strtol uses it.
		input = strtol(input_buf, &first_unconverted_character, 10); // radix (aka base) = 10

		// possible cases:
		//   first_unconverted_character pointer is equal to input_buffer (pointer), so nothing was converted
		//   first_unconverted_character pointer is set, but it isn't \n so something else was encountered that couldn't be converted
		//   errno is set to ERANGE if number is not in range for long
		//   number is not in range to cast to int
		//   number is negative

		if (first_unconverted_character == input_buf ||
			(*first_unconverted_character && *first_unconverted_character != '\n')) {
			puts("Unable to properly convert input. Please try again.");
			continue;
		}

		if (errno == ERANGE || input < 0) {
			printf("Input must be a positive integer less than or equal to %d. Please try again.\n", INT_MAX);
			continue;
		}

		// success
		(*input_value) = input;
		break;
	}
	return 0;
}

/**
* @brief Get a single character input from stdin. Force the user to retry until input is valid.
* @param *input_value Where to put successful input
* @param *prompt String to display for each prompt
* @return Returns 0 if successful, non-zero if failed
*/
int getCharInput(char* input_value, char* prompt) {
	char input_buf[16];
	while (true) {
		printf(prompt);

		// get up to 16 characters of input or up to newline
		// fgets returns NULL in the event reading input failed
		if (fgets(input_buf, 16, stdin) == NULL) {
			// Failed to read input
			return -1;
		}

		// must be one character that is not '\n' or ' ' or '\t' or NUL
		if (input_buf[0] == '\n' || input_buf[0] == ' ' || input_buf[0] == '\t' || input_buf[0] == '\0') {
			puts("Must enter a non-whitespace character. Please try again.");
			continue;
		}

		// must be no more than one character before the '\n'
		if (input_buf[1] != '\n') {
			puts("Must enter exactly one character. Please try again.");
			continue;
		}

		// success
		(*input_value) = input_buf[0];
		break;
	}
	return 0;
}

/**
* @brief Display a right-angle triangle
* @param n Number of rows to output
* @param print_char What character to use to display the triangle
*/
void printRightTriangle(int n, bool invert_vertical, bool invert_horizontal, char print_char) {
	for (int i = 1; i <= n; i++) {
		if (invert_horizontal) {
			// need to print leading spaces
			for (int j = 0; j < (invert_vertical ? i - 1 : n - i); j++) {
				putchar(' ');
			}
		}
		for (int j = 0; j < (invert_vertical ? n - i + 1 : i); j++) {
			putchar(print_char);
		}
		putchar('\n');
	}
}

/**
* @brief Display an isosceles triangle
* @param n Number of rows to output
* @param print_char What character to use to display the triangle
*/
void printIsoscelesTriangle(int n, bool invert_vertical, char print_char) {
	for (int i = 1; i <= n; i++) {
		// need to print leading spaces
		for (int j = 0; j < (invert_vertical ? i - 1 : n - i); j++) {
			putchar(' ');
		}
		for (int j = 0; j < (invert_vertical ? (n - i) * 2 + 1 : (i - 1) * 2 + 1); j++) {
			putchar(print_char);
		}
		putchar('\n');
	}
}


/**
* @brief Display a diamond with n increasing rows followed by n - 1 decreasing rows
* @param n Size to display (n * 2 - 1 rows will be output).
* @param print_char What character to use to display the diamond
*/
void printDiamond(int n, char print_char) {
	for (int i = -n; i <= n; i++) {
		// need to print leading spaces
		for (int j = 0; j < abs(i); j++) {
			putchar(' ');
		}
		for (int j = 0; j < ((n - abs(i) - 1) * 2 + 1); j++) {
			putchar(print_char);
		}
		putchar('\n');
	}
}

int main() {
	// get int input for length
	int size;
	if (getIntInput(&size, "Enter size to display (integer): ") != 0) {
		fputs("Failed to get input.", stderr);
		return EXIT_FAILURE;
	}

	// get int input for shape
	// right triangle, isosceles triangle, or diamond (inverted or not), using a specified character.
	int shape = 0;
	do {
		puts("Shapes:");
		puts("1 - Right triangle");
		puts("2 - Right triangle (vertical inversion)");
		puts("3 - Right triangle (horizontal inversion)");
		puts("4 - Right triangle (double inversion)");
		puts("5 - Isosceles triangle");
		puts("6 - Isosceles (vertical inversion)");
		puts("7 - Diamond");
		if (getIntInput(&shape, "Select: ") != 0) {
			fputs("Failed to get input.", stderr);
			return EXIT_FAILURE;
		}
	} while (shape < 1 || shape > 7);
	
	// get char input for character
	char output_symbol;
	if (getCharInput(&output_symbol, "Enter character to display: ") != 0) {
		fputs("Failed to get input.", stderr);
		return EXIT_FAILURE;
	}

	// based on selection, output the shape.
	switch (shape) {
		case 1:
			printRightTriangle(size, false, false, output_symbol);
			break;
		case 2:
			printRightTriangle(size, true, false, output_symbol);
			break;
		case 3:
			printRightTriangle(size, false, true, output_symbol);
			break;
		case 4:
			printRightTriangle(size, true, true, output_symbol);
			break;
		case 5:
			printIsoscelesTriangle(size, false, output_symbol);
			break;
		case 6:
			printIsoscelesTriangle(size, true, output_symbol);
			break;
		default:
			printDiamond(size, output_symbol);
			break;
	}

	return EXIT_SUCCESS;
}