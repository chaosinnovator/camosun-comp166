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
* If number of provided arguments is invalid, output a help description instead.
*
*/
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

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
	//   errno is set to ERANGE if number is not in range for long

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
	if (year <= 0 || month < 0 || month >= 12) {
		return -1;
	}

	int days;
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

	// Could also use this bitwise one-liner, about the same speed. Left it here since I think it's elegant (though less readable)
	/*
	int days = 30 + (((month + 1) & 1) ^ (month + 1) >> 3); // low bit for month+1 is 1 for months with 31 days for the first 7 months, then low bit 0
	if (month == 1) { // February. -2 days normally or -1 day if leap year.
		days += (year % 4 == 0 && (year % 100 != 0 || year % 400 == 0)) ? -1 : -2;
	}
	*/

	*result = days;

	return 0;
}

void outputHelp() {
	puts("Determines the number of days in a provided month\n");
	puts("Usage: Lab2_Task2 [year] [month_number]");
	puts("  [year]          The year.");
	puts("  [month_number]  The month number (0=January, 11=December).\n");
}

int main(int argc, char* argv[]) {
	// check for correct number of args
	// [program name] [year] [month]
	if (argc != 3) {
		outputHelp();
		return EXIT_FAILURE;
	}

	// convert input values:
	// if can't convert value to int (parseDouble returns non-zero if error):
	//   print error message and return fail
	int input_year;
	if (parseInt(argv[1], &input_year) != 0) {
		fputs("Unable to parse provided [year].", stderr);
		return EXIT_FAILURE;
	}

	int input_month;
	if (parseInt(argv[2], &input_month) != 0) {
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
	// tidier to use an array like month_names[input_month] to print the month name
	// using switch statement instead since we haven't learned about arrays beyond argv[] yet.
	char* month_name;
	switch (input_month) {
		case 0:
			month_name = "January";
			break;
		case 1:
			month_name = "February";
			break;
		case 2:
			month_name = "March";
			break;
		case 3:
			month_name = "April";
			break;
		case 4:
			month_name = "May";
			break;
		case 5:
			month_name = "June";
			break;
		case 6:
			month_name = "July";
			break;
		case 7:
			month_name = "August";
			break;
		case 8:
			month_name = "September";
			break;
		case 9:
			month_name = "October";
			break;
		case 10:
			month_name = "November";
			break;
		case 11:
			month_name = "December";
			break;
		default: // this is unreachable since daysInMonth checks that input_month is in range.
			month_name = "Unknown Month";
			break;
	}
	printf("%s %04d has %d days.\n", month_name, input_year, days);

	return EXIT_SUCCESS;
}