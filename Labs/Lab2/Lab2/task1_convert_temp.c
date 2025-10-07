/*
* 2025F COMP-166 Lab 2 - Thomas Boland - C0556991
*
* This program converts a given temperature to Celcius, Kelvin, and Fahrenheit
*
* The program accepts two arguments from the command line:
* [value_to_convert] [unit]
* where unit is C, c, K, k, F, or f
* 
* If number of provided arguments is invalid, output a help description instead.
* 
*/
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

typedef enum {
	Celcius,
	Kelvin,
	Fahrenheit,
} TemperatureUnit;

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
	puts("                        C, c = Celcius");
	puts("                        K, k = Kelvin");
	puts("                        F, f = Fahrenheit\n");
}

int main(int argc, char* argv[]) {
	// check for correct number of args
	// [program name] [temp_value] [temp_unit]
	if (argc != 3) {
		outputHelp();
		return EXIT_FAILURE;
	}

	// convert input values:
	// if can't convert value to double (parseDouble returns non-zero if error):
	//   print error message and return fail
	double input_temp;
	if (parseDouble(argv[1], &input_temp) != 0) {
		fputs("Unable to parse provided [temperature_value].", stderr);
		return EXIT_FAILURE;
	}

	// if argv[2] is 0 characters long or more than 1 character long, can't parse.
	if (argv[2][0] == '\0' || argv[2][1] != '\0') {
		fputs("Unable to parse provided [unit].", stderr);
		return EXIT_FAILURE;
	}

	// if unit not acceptable value (converting to enum):
	//   print error message and return fail
	TemperatureUnit input_temp_unit;
	switch (argv[2][0]) {
		case 'C':
		case 'c':
			input_temp_unit = Celcius;
			break;
		case 'K':
		case 'k':
			input_temp_unit = Kelvin;
			break;
		case 'F':
		case 'f':
			input_temp_unit = Fahrenheit;
			break;
		default:
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
		fputs("Brrr! The provided [temperature_value] is below absolute zero.", stderr);
		return EXIT_FAILURE;
	}

	// convert temp_kelvin to temp_celcius and temp_fahrenheit and output results.
	// \xF8 is the degree symbol.
	printf("%.1f\xF8\C, %.1f\xF8\K, %.1f\xF8\F", kelvinToCelcius(temp_kelvin), temp_kelvin, kelvinToFahrenheit(temp_kelvin));

	return EXIT_SUCCESS;
}