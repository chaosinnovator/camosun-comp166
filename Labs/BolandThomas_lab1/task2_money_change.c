/*
* 2025F COMP-166 Lab 1 - Thomas Boland - C0556991
* 
* This program calculates the optimal number of each denomination
* of bills/coins for a given amount of cash.
* 
* For the purpose of the lab, the denominations to be used are
* $10 bills, toonies ($2), quarters ($0.25), dimes ($0.10), and pennies ($0.01).
* However, the program is designed to make adding more denominations straightforward.
* 
* The program takes input from the user for the number of CENTS
* for which to calculate change denominations.
*/
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>

// Make it easy to add/remove denominations if name and value are in the same array
// instead of having to maintain two parallel arrays that must always be the same length
typedef struct {
	int valueCents;
	char name[12];
} Denomination;

// NEW DENOMINATIONS may be added to this array. Make sure N_DENOMINATIONS is adjusted accordingly.
// Array of denomination definitions must be sorted from largest to smallest value
#define N_DENOMINATIONS 5
Denomination denominations[N_DENOMINATIONS] = {
	{1000, "$10 bills"},
	{ 200, "toonies"},
	{  25, "quarters"},
	{  10, "dimes"},
	{   1, "pennies"},
};

// These constants are used to make outputting a pretty table easier by using fixed-size arrays
// instead of keeping track of variable length arrays
#define MAX_N_TABLE_COLUMNS 8
#define MAX_TABLE_WIDTH 100
#define MAX_TABLE_VALUE_LENGTH 32

/// <summary>
/// Safely get a positive integer number of cents as input. Validate
/// and require the user to try again if invalid.
/// </summary>
/// <param name="*input">Where to put the input value</param>
/// <returns>Returns 0 if successful, non-zero if not successful.</returns>
int getMoneyInput(int *input) {
	char input_buf[128];
	long input_value;
	int input_success = 0;
	while (!input_success) {
		printf("Enter amount of cash (in CENTS): ");

		// get up to 128 characters of input or up to newline
		// fgets returns NULL in the event reading input failed
		if (fgets(input_buf, 128, stdin) == NULL) {
			// Failed to read input
			return -1;
		}

		char* first_unconverted_character;
		errno = 0; // I'm not a fan of the errno pattern, but strtol uses it.
		input_value = strtol(input_buf, &first_unconverted_character, 10); // radix (aka base) = 10

		// possible cases:
		//   first_unconverted_character pointer is equal to input_buffer (pointer), so nothing was converted
		//   first_unconverted_character pointer is set, but it isn't \n so something else was encountered that couldn't be converted
		// 
		//   errno is set to ERANGE if number is not in range for long
		//   number is not in range to cast to int
		//   number is negative

		if (first_unconverted_character == input_buf ||
			(*first_unconverted_character && *first_unconverted_character != '\n')) {
			puts("Unable to properly convert input. Please try again.");
			continue;
		}

		if (errno == ERANGE || input_value > INT_MAX || input_value < 0) {
			printf("Input must be a positive integer less than or equal to %d. Please try again.\n", INT_MAX);
			continue;
		}

		input_success = 1;
	}
	*input = (int)input_value;

	return 0;
}

/// <summary>
/// Calculate the amount of each denomination to make up money_cents
/// </summary>
/// <param name="money_cents">The money to compute denominations of. Must be in cents.</param>
/// <param name="denominations[]">Array of denominations to use. Must be sorted from largest to smallest, this will not be checked.</param>
/// <param name="denom_quanitities[]">Where to put the results. Array must be same length as denominations</param>
/// <returns>Returns 0 if successful, non-zero if not successful.</returns>
int calculateDenominations(int money_cents, Denomination denominations[], int denom_quantities[], int num_denominations) {
	// First, check that money_cents is positive
	if (money_cents < 0) {
		return -1;
	}

	for (int d = 0; d < num_denominations; d++) {
		denom_quantities[d] = money_cents / denominations[d].valueCents; // Integer division is intentional, since need whole number result
		money_cents = money_cents % denominations[d].valueCents;
	}

	return 0;
}

int main() {
	// process:
	// getInput()
	// check if there was an error getting the input
	// calculateChange(mass[0] mass[1])
	// check if there was an error during the calculation
	// output the results in a pretty table

	int money_cents;
	if (getMoneyInput(&money_cents)) {
		fputs("Failed to read input. Exiting.", stderr);
		return EXIT_FAILURE;
	}

	int denom_quantities[N_DENOMINATIONS];
	if (calculateDenominations(money_cents, denominations, &denom_quantities, N_DENOMINATIONS)) {
		fputs("Despite input validation, calculation failed. Provided money value is negative. Exiting.", stderr);
		return EXIT_FAILURE;
	}

	char table_headers[2][MAX_TABLE_VALUE_LENGTH] = {"Denomination", "Qty"};
	// convert to table_values
	char table_values[N_DENOMINATIONS * 2][MAX_TABLE_VALUE_LENGTH];
	char buffer[16];
	for (int d = 0; d < N_DENOMINATIONS; d++) {
		strncpy_s(table_values[d * 2], MAX_TABLE_VALUE_LENGTH, denominations[d].name, _TRUNCATE);
		sprintf_s(buffer, 16, "%d", denom_quantities[d]); // int to string
		strncpy_s(table_values[d * 2 + 1], MAX_TABLE_VALUE_LENGTH, buffer, _TRUNCATE);
	}

	if (outputTable(table_values, table_headers, N_DENOMINATIONS, 2)) {
		fputs("Failed to output the table. Too many columns or exceeds max width. Exiting.", stderr);
		return EXIT_FAILURE;
	};

	printf("Press enter to exit...");
	getchar();
	
	return EXIT_SUCCESS;
}

/************************************************************************
*                                                                       *
* The below functions are for outputting a pretty data table to stdout. *
* This functionality is an extension that was not required for the lab, *
* and utility functions like this would typically be in a separate      *
* library module. They are just included here for the sake of the lab.  *
*                                                                       *
*************************************************************************/

/// <summary>
/// Print count characters c to stdout
/// </summary>
/// <param name="c">The character to print</param>
/// <param name="count">Number of times to print the character c</param>
/// <returns>Returns 0 if successful, non-zero if not successful.</returns>
int printNChars(const char c, const int count) {
	for (int i = 0; i < count; i++) {
		if (putchar(c) == EOF) {
			return -1; // printing failed
		}
	}

	return 0;
}

/// <summary>
/// Prints the provided values in a table.
/// </summary>
/// <param name="table_values">Array of strings in order of rows "stuck together." Must have length rows*columns</param>
/// <param name="table_headers">Array of strings to be printed as headers for the corresponding column</param>
/// <param name="num_rows">How many rows represented by table_values</param>
/// <param name="num_columns">How many columns per row of values</param>
/// <returns>Returns 0 if successful, non-zero if not successful.</returns>
int outputTable(const char table_values[][MAX_TABLE_VALUE_LENGTH], const char table_headers[][MAX_TABLE_VALUE_LENGTH], int num_rows, int num_columns) {
	// Table formated like this:
	// 
	//   Denomination   Qty
	// ----------------------
	// | $10 bills    | 0   |
	// | toonies      | 0   |
	// | quarters     | 0   |
	// | dimes        | 0   |
	// | pennies      | 5   |
	// ----------------------
	// table_value should be of length num_rows*num_columns
	// table_headers should be of length num_columns
	// process:
	//   determine width of each column (max str length)
	//   for each table_header:
	//     output space, space, value, space (plus space * max_length-cell_length)
	//   output \n
	//   output horizontal line
	//   for each value:
	//     output left border, space, value, space (plus space * max_length-cell_length)
	//     if end of row, output right border and \n
	//   output horizontal line

	// limitation: num_columns must be <= MAX_N_TABLE_COLUMNS
	int column_width[MAX_N_TABLE_COLUMNS];
	if (num_columns > MAX_N_TABLE_COLUMNS) {
		return -1;
	}

	// initialize array
	for (int i = 0; i < MAX_N_TABLE_COLUMNS; i++) {
		column_width[i] = 0;
	}

	// find column widths
	int col;
	for (int i = 0; i < num_rows * num_columns; i++) {
		col = i % num_columns;
		column_width[col] = (column_width[col] < strlen(table_values[i])) ? strlen(table_values[i]) : column_width[col];
		// also check the header widths on the first row
		if (i < num_columns) {
			column_width[col] = (column_width[col] < strlen(table_headers[i])) ? strlen(table_headers[i]) : column_width[col];
		}
	}

	// check total width is less than MAX_TABLE_WIDTH
	int width = 1; // extra +1 for right border
	for (int i = 0; i < num_columns; i++) {
		width += column_width[i] + 3; // +3: border and two spaces
	}
	if (width > MAX_TABLE_WIDTH) {
		return -1;
	}

	// output top border, headers, header border, values, bottom border
	int row;
	int padding_width;
	for (int i = 0; i < (num_rows + 4) * num_columns; i++) {
		row = i / num_columns;
		col = i % num_columns;
		if (row == 0) {
			// printing top border
			if (col == 0) {
				putchar('\xDA');
			}
			else {
				putchar('\xC2');
			}
			printNChars('\xC4', column_width[col] + 2);
			if (col == (num_columns - 1)) {
				printf("\xBF\n"); // end of row, print corner and \n
			}
			continue;
		}
		if (row == 1) {
			// printing header values
			padding_width = column_width[col] - strlen(table_headers[col]);
			printf("\xB3 ");
			printNChars(' ', padding_width);
			printf("%s ", table_headers[col]);
			if (col == (num_columns - 1)) {
				printf("\xB3\n"); // end of row, print border and \n
			}
			continue;
		}
		if (row == 2) {
			// printing header border
			if (col == 0) {
				putchar('\xC3');
			}
			else {
				putchar('\xC5');
			}
			printNChars('\xC4', column_width[col] + 2);
			if (col == (num_columns - 1)) {
				printf("\xB4\n"); // end of row, print corner and \n
			}
			continue;
		}
		if (row == (num_rows + 3)) {
			// printing bottom border
			if (col == 0) {
				putchar('\xC0');
			}
			else {
				putchar('\xC1');
			}
			printNChars('\xC4', column_width[col] + 2);
			if (col == (num_columns - 1)) {
				printf("\xD9\n"); // end of row, print corner and \n
			}
			continue;
		}
		// printing values
		padding_width = column_width[col] - strlen(table_values[i - num_columns * 3]);
		printf("\xB3 ");
		printNChars(' ', padding_width);
		printf("%s ", table_values[i - num_columns * 3]);
		if (col == (num_columns - 1)) {
			printf("\xB3\n"); // end of row, print border and \n
		}
	}

	return 0;
}