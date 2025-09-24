/*
* 2025F COMP-166 Lab 2 - Thomas Boland - C0556991
*
* This program converts a given temperature to Celcius, Kelvin, and Fahrenheit
*
* The program accepts two arguments from the command line:
* [value_to_convert] [unit]
* where unit is C, c, Celcius, celcius, K, k, Kelvin, kelvin, F, f, Fahrenheit, or fahrenheit
* 
* If provided arguments are invalid or any argument is /?, output a help description instead.
* 
*/
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>

typedef struct {
	char* value_str;
	char* unit_str;
	char* unknown_arg;
	bool help_flag;
} Arguments;

typedef enum {
	Celcius,
	Kelvin,
	Fahrenheit,
} TemperatureUnit;

// These constants are used to make outputting a pretty table easier by using fixed-size arrays
// instead of keeping track of variable length arrays
#define MAX_N_TABLE_COLUMNS 8
#define MAX_TABLE_WIDTH 100
#define MAX_TABLE_VALUE_LENGTH 32

/// <summary>
/// Check that two arguments were provided. Additionally, check if help was requested or if
/// invalid arguments were provided
/// <param name="*arg_struct">Where to put result</param>
/// <returns>Returns 0 if successful, non-zero if not successful.</returns>
int parseArgs(int argc, char* argv[], Arguments* arg_struct) {
	// check each provided arg.
	// if /?, set help_flag to true and return 0
	// if first char is not / and value_str hasn't been set yet, set it and continue
	// if first char is not / unit_str hasn't been set yet, set it and continue.
	// if there are any other arguments, set unknown_arg, set help_flag to true, return -1
	// if not both value_str and unit_str were set, set help_flag to true. This covers the case if no arguments provided.

	for (int i = 1; i < argc; i++) // first arg is the program name. skip it.
	{
		if (strcmp(argv[i], "/?") == 0) {
			(*arg_struct).help_flag = true;
			return 0;
		}
		if (argv[i][0] != '/' && (*arg_struct).value_str == NULL) {
			(*arg_struct).value_str = argv[i];
			continue;
		}
		if (argv[i][0] != '/' && (*arg_struct).unit_str == NULL) {
			(*arg_struct).unit_str = argv[i];
			continue;
		}
		(*arg_struct).unknown_arg = argv[i];
		(*arg_struct).help_flag = true;
		return -1;
	}
	// only need to check unit_str to determine if both were set, since they get set in order
	// if unit_str is set, value_str must have also been set already.
	if ((*arg_struct).unit_str == NULL) {
		(*arg_struct).help_flag = true;
	}
	return 0;
}

/// <summary>
/// Safely parse a double value from a string.
/// </summary>
/// <param name="*str">String to parse</param>
/// <param name="*result">Where to put result</param>
/// <returns>Returns 0 if successful, non-zero if not successful.</returns>
int parseDouble(char* str, double* result) {
	double parsed_value;

	char* first_unconverted_character;
	errno = 0; // I'm not a fan of the errno pattern, but strtod uses it.
	parsed_value = strtod(str, &first_unconverted_character);

	// possible cases:
	//   first_unconverted_character pointer is equal to str (pointer), so nothing was converted
	//   first_unconverted_character pointer is set, but it isn't \n so something else was encountered that couldn't be converted
	// 
	//   errno is set to ERANGE if number is not in range for double

	if (first_unconverted_character == str ||
		(*first_unconverted_character && *first_unconverted_character != '\n')) {
		return -1;
	}

	if (errno == ERANGE) {
		return -1;
	}

	*result = parsed_value;

	return 0;
}

/// <summary>
/// Safely parse the correct TemperatureUnit enum value from a string.
/// </summary>
/// <param name="*str">String to parse</param>
/// <param name="*result">Where to put result</param>
/// <returns>Returns 0 if successful, non-zero if not successful.</returns>
int parseUnit(char* str, TemperatureUnit* result) {
	// Acceptable values: C, c, Celcius, celcius, K, k, Kelvin, kelvin, F, f, Fahrenheit, or fahrenheit
	// an alternate option would be to convert str to lowercase first for half as many "strcmp"s. still need to loop through str 7x
	// or, use a hash function to convert str to an int then compare with pre-computed hashed of each acceptable value for only 1x loop of str.
	if (strcmp(str, "C") == 0 || strcmp(str, "c") == 0 || strcmp(str, "Celcius") == 0 || strcmp(str, "celcius") == 0) {
		*result = Celcius;
		return 0;
	}
	if (strcmp(str, "K") == 0 || strcmp(str, "k") == 0 || strcmp(str, "Kelvin") == 0 || strcmp(str, "kelcius") == 0) {
		*result = Kelvin;
		return 0;
	}
	if (strcmp(str, "F") == 0 || strcmp(str, "f") == 0 || strcmp(str, "Fahrenheit") == 0 || strcmp(str, "farhrenheit") == 0) {
		*result = Fahrenheit;
		return 0;
	}

	return -1;
}

double kelvinToCelcius(double kelvin) {
	return kelvin - 273.15;
}

double celciusToKelvin(double celcius) {
	return celcius + 273.15;
}

double kelvinToFahrenheit(double kelvin) {
	return (kelvin - 273.15) * 1.8 + 32;
}

double fahrenheitToKelvin(double fahrenheit) {
	return (fahrenheit - 32) / 1.8 + 273.15;
}

void outputHelp() {
	puts("Converts an input temperature to Celcius, Kelvin, and Fahrenheit.\n");
	puts("Usage: Lab2_Task1 [temperature_value] [unit]");
	puts("  [temperature_value]  The temperature to convert.");
	puts("  [unit]               The unit of [temperature_value]");
	puts("                        C, c, Celcius, or celcius");
	puts("                        K, k, Kelvin, kelvin");
	puts("                        F, f, Fahrenheit, fahrenheit");
	puts("  /?                   Display this help page.\n");
}

int main(int argc, char* argv[]) {
	// parse input argument strings. returns -1 (and sets help flag) if error
	// if error:
	//   Unknown argument "[arg]"
	Arguments arg_struct = {
		NULL, // value_str
		NULL, // unit_str
		NULL, // unknown_arg
		false, // help_flag
	};
	if (parseArgs(argc, argv, &arg_struct) != 0) {
		// there is an unknown argument.
		printf_s("Unknown argument \"%s\"\n", arg_struct.unknown_arg);
	}


	// if args.help == true
	//   Print help text
	//   return success
	if (arg_struct.help_flag) {
		outputHelp();
		return EXIT_SUCCESS;
	}

	// convert input values:
	// if can't convert value to double:
	//   print error message and return fail
	// if unit not acceptable value (convert to enum):
	//   print error message and return fail
	double input_temp;
	if (parseDouble(arg_struct.value_str, &input_temp) != 0) {
		fputs("Unable to parse provided [temperature_value].", stderr);
		return EXIT_FAILURE;
	}

	TemperatureUnit input_temp_unit;
	if (parseUnit(arg_struct.unit_str, &input_temp_unit) != 0) {
		fputs("Unable to parse provided [unit].", stderr);
		return EXIT_FAILURE;
	}

	// convert input to Kelvin -> temp_kelvin
	double temp_kelvin;
	switch (input_temp_unit) {
		case Celcius:
			temp_kelvin = celciusToKelvin(input_temp);
			break;
		case Fahrenheit:
			temp_kelvin = fahrenheitToKelvin(input_temp);
			break;
		case Kelvin:
		default: // only remaining case is Kelvin
			temp_kelvin = input_temp;
			break;
	}

	// if temp_kelvin < 0:
	//   print error message and return fail
	if (temp_kelvin < 0) {
		fputs("Brrr! The provided [temperature_value] is below absolute zero, which is impossible by definition.", stderr);
		return EXIT_FAILURE;
	}

	// convert temp_kelvin to temp_celcius and temp_fahrenheit
	// output the results in a pretty table

	char table_headers[1][MAX_TABLE_VALUE_LENGTH] = { "Converted Temperature" };
	// convert to table_values
	char table_values[3][MAX_TABLE_VALUE_LENGTH];
	char buffer[32];
	sprintf_s(buffer, 32, "%.1f\xF8\C", kelvinToCelcius(temp_kelvin)); // double to string
	strncpy_s(table_values[0], MAX_TABLE_VALUE_LENGTH, buffer, _TRUNCATE);

	sprintf_s(buffer, 32, "%.1f\xF8\K", temp_kelvin);
	strncpy_s(table_values[1], MAX_TABLE_VALUE_LENGTH, buffer, _TRUNCATE);

	sprintf_s(buffer, 32, "%.1f\xF8\F", kelvinToFahrenheit(temp_kelvin));
	strncpy_s(table_values[2], MAX_TABLE_VALUE_LENGTH, buffer, _TRUNCATE);

	if (outputTable(table_values, table_headers, 3, 1)) {
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