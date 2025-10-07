/*
* 2025F COMP-166 Lab 2 - Thomas Boland - C0556991
*
* This program determines the number of days in a given month.
* For simplicity for the sake of the lab, I've made the following
* assumptions:
*  - Using the Gregorian calendar
*  - Assume Gregorian calendar from year 1AD onwards and that geography doesn't matter
*      (Gregorian calendar only existed since ~October 1582, but this varied significantly
*       by country and even within country. Some countries, i.e. Sweeden, had a more complicated
*       shift from the Julian calendar to the Gregorian calendar.)
* 
*
* The program accepts two arguments from the command line:
* [year] [month]
* Where year >= 1 and 0 <= month <= 11
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
	char* year_str;
	char* month_str;
	char* unknown_arg;
	bool help_flag;
} Arguments;

#define N_MONTHS 12
const char* month_names[N_MONTHS] = {
	"January", "February", "March", "April", "May", "June",
	"July", "August", "September", "October", "November", "December",
};

/// <summary>
/// Check that two arguments were provided. Additionally, check if help was requested or if
/// invalid arguments were provided
/// <param name="*arg_struct">Where to put result</param>
/// <returns>Returns 0 if successful, non-zero if not successful.</returns>
int parseArgs(int argc, char* argv[], Arguments* arg_struct) {
	// check each provided arg.
	// if /?, set help_flag to true and return 0
	// if first char is not / and year_str hasn't been set yet, set it and continue
	// if first char is not / month_str hasn't been set yet, set it and continue.
	// if there are any other arguments, set unknown_arg, set help_flag to true, return -1
	// if not both year_str and month_str were set, set help_flag to true. This covers the case if no arguments provided.

	for (int i = 1; i < argc; i++) // first arg is the program name. skip it.
	{
		if (strcmp(argv[i], "/?") == 0) {
			(*arg_struct).help_flag = true;
			return 0;
		}
		if (argv[i][0] != '/' && (*arg_struct).year_str == NULL) {
			(*arg_struct).year_str = argv[i];
			continue;
		}
		if (argv[i][0] != '/' && (*arg_struct).month_str == NULL) {
			(*arg_struct).month_str = argv[i];
			continue;
		}
		(*arg_struct).unknown_arg = argv[i];
		(*arg_struct).help_flag = true;
		return -1;
	}
	// only need to check month_str to determine if both were set, since they get set in order
	// if month_str is set, year_str must have also been set already.
	if ((*arg_struct).month_str == NULL) {
		(*arg_struct).help_flag = true;
	}
	return 0;
}

/// <summary>
/// Safely parse an int value from a string.
/// </summary>
/// <param name="*str">String to parse</param>
/// <param name="*result">Where to put result</param>
/// <returns>Returns 0 if successful, non-zero if not successful.</returns>
int parseInt(char* str, int* result) {
	int parsed_value;

	char* first_unconverted_character;
	errno = 0; // I'm not a fan of the errno pattern, but strtod uses it.
	parsed_value = strtol(str, &first_unconverted_character, 10);

	// possible cases:
	//   first_unconverted_character pointer is equal to str (pointer), so nothing was converted
	//   first_unconverted_character pointer is set, but it isn't \n so something else was encountered that couldn't be converted
	// 
	//   errno is set to ERANGE if number is not in range for double

	if (first_unconverted_character == str ||
		(*first_unconverted_character && *first_unconverted_character != '\n')) {
		return -1;
	}

	if (errno == ERANGE || parsed_value > INT_MAX || parsed_value < INT_MIN) {
		return -1;
	}

	*result = parsed_value;

	return 0;
}

/// <summary>
/// Determine the number of days in a month
/// </summary>
/// <param name="year">Year number</param>
/// <param name="month">Month number</param>
/// <param name="*result">Where to put result</param>
/// <returns>Returns 0 if successful, non-zero if not successful.</returns>
int daysInMonth(int year, int month, int* result) {
	// check validity of year number and month
	if (year <= 0 || month < 0 || month >= N_MONTHS) {
		return -1;
	}

	// mth  mth+1     days
	// 0000 0001  jan 31
	// 0001 0010  feb 28 or 29
	// 0010 0011  mar 31
	// 0011 0100  apr 30
	// 0100 0101  may 31
	// 0101 0110  jun 30
	// 0110 0111  jul 31
	// 
	// 0111 1000  aug 31
	// 1000 1001  sep 30
	// 1001 1010  oct 31
	// 1010 1011  nov 30
	// 1011 1100  dec 31
	int days = 30 + (((month + 1) & 1) ^ (month + 1) >> 3); // low bit for month+1 is 1 for months with 31 days for the first 7 months, then low bit 0
	if (month == 1) { // February. -2 days normally or -1 day if leap year.
		days += (year % 4 == 0 && (year % 100 != 0 || year % 400 == 0)) ? -1 : -2;
	}

	// Can use the following switch statement, but I think the bitwise one-liner is neat. They're about the same speed.
	/*
	switch (month) {
		case 1:
			days = (year % 4 == 0 && (year % 100 != 0 || year % 400 == 0)) ? 29 : 28;
			break;
		case 3: // april
		case 5: // june
		case 8: // september
		case 10: // november
			days = 30;
			break;
		default:
			days = 31;
	}
	*/

	*result = days;

	return 0;
}

void outputHelp() {
	puts("Determines the number of days in a provided month\n");
	puts("Usage: Lab2_Task2 [year] [month_number]");
	puts("  [year]          The year.");
	puts("  [month_number]  The month number (0=January, 11=December).");
	puts("  /?              Display this help page.\n");
}

int main(int argc, char* argv[]) {
	// parse input argument strings. returns -1 (and sets help flag) if error
	// if error:
	//   Unknown argument "[arg]"
	Arguments arg_struct = {
		NULL, // year_str
		NULL, // month_str
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
	// if can't convert value to int:
	//   print error message and return fail
	int input_year;
	if (parseInt(arg_struct.year_str, &input_year) != 0) {
		fputs("Unable to parse provided [year].", stderr);
		return EXIT_FAILURE;
	}

	int input_month;
	if (parseInt(arg_struct.month_str, &input_month) != 0) {
		fputs("Unable to parse provided [month_number].", stderr);
		return EXIT_FAILURE;
	}

	// get number of days
	int days;
	if (daysInMonth(input_year, input_month, &days) != 0) {
		fputs("Provided year must be at least 1. Provided month must be between 0 and 11.", stderr);
		return EXIT_FAILURE;
	}

	// output
	printf("%s %d has %d days.\n", month_names[input_month], input_year, days);

	return EXIT_SUCCESS;
}