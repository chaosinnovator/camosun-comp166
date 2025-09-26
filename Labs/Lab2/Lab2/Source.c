#include <stdio.h>
#include <stdlib.h>

float celciusToKelvin(float celcius) {
	return celcius + 273.15;
}

float kelvinToCelcius(float kelvin) {
	return kelvin - 273.15;
}

float celciusToFahrenheit(float celcius) {
	// F = C * 1.8 + 32
	return celcius * 1.8 + 32.0;
}

float fahrenheitToCelcius(float fahrenheit) {
	return (fahrenheit - 32.0) / 1.8;
}

int main(int argc, char* argv[]) {
	// check input is valid
	if (argc != 3) {
		puts("Wrong number of args");
		return 1;
	}
	float input;
	char unit;
	input = strtof(argv[1], NULL);
	unit = argv[2][0];
	//input = 100.0;
	//unit = 'k';
	// check which unit is input and convert to celcius
	float temp_celcius;
	switch (unit) {
		case 'C':
		case 'c':
			temp_celcius = input;
			break;
		case 'F':
		case 'f':
			temp_celcius = fahrenheitToCelcius(input);
			break;
		case 'K':
		case 'k':
			temp_celcius = kelvinToCelcius(input);
			break;
		default:
			puts("Wrong unit");
			return 1;
	}


	// convert celcius -> kelvin
	float kelvin = celciusToKelvin(temp_celcius);
	// convert celcius -> fahrenheit
	float fehrenheit = celciusToFahrenheit(temp_celcius);;

	// print
	printf("%.1f\xF8\K, %.1f C, %.1f F\n", kelvin, temp_celcius, fehrenheit);
	return 0;
}