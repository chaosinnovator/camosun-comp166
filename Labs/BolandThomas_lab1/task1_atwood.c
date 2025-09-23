/*
* 2025F COMP-166 Lab 1 - Thomas Boland - C0556991
* 
* This program calculates the acceleration of weights in
* an Atwood Machine (see: https://en.wikipedia.org/wiki/Atwood_machine).
*
* The program takes input from the user for the mass of each weight as an integer
* and outputs the acceleration of the weights in m/s^2 to stdout.
*/
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

#define N_MASS_INPUTS 2
#define GRAVITY 9.806 // m/s^2

/// <summary>
/// Safely get N_MASS_INPUTS number of positive inputs. Validate each input as
/// provided and require the user to try again if invalid input is entered.
/// </summary>
/// <param name="*inputs">Where to put the input values</param>
/// <param name="n_inputs">How many inputs to get</param>
/// <returns>Returns 0 if successful, non-zero if not successful.</returns>
int getMassInputs(int inputs[], int n_inputs) {
	for (short i = 0; i < n_inputs; i++) {
		char input_buf[128];
		long input_value;
		int input_success = 0;
		while (!input_success) {
			printf("Enter mass %d (unit doesn't matter, but both masses must be in the same unit): ", i + 1);

			// get up to 128 characters of input or up to newline
			// fgets returns NULL in the event reading input failed
			if (fgets(input_buf, 128, stdin) == NULL) {
				// Failed to read input
				return -1;
			}

			char *first_unconverted_character;
			errno = 0; // I'm not a fan of the errno pattern, but strtol uses it.
			input_value = strtol(input_buf, &first_unconverted_character, 10); // radix (aka base) = 10

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

			if (errno == ERANGE || input_value > INT_MAX || input_value < 0) {
				printf("Input must be a positive integer less than or equal to %d. Please try again.\n", INT_MAX);
				continue;
			}

			input_success = 1;
		}
		inputs[i] = (int)input_value;
	}
	return 0;
}

/// <summary>
/// Calculate the acceleration of weights in an Atwood Machine (see: https://en.wikipedia.org/wiki/Atwood_machine).
/// </summary>
/// <param name="m1, m2">The mass of each weight. Must be positive.</param>
/// <param name="*acceleration">Where to put the return value (m/s^2)</param>
/// <returns>Returns 0 if successful, non-zero if not successful.</returns>
int calculateAtwoodAcceleration(int m1, int m2, double *acceleration) {
	// Acceleration of the weights is given by the equation
	// a = g*(m1-m2)/(m1+m2)
	
	// First, check that both m1 and m2 are positive
	if (m1 < 0 || m2 < 0) {
		return -1;
	}

	*acceleration = GRAVITY * (double)(m1 - m2) / (double)(m1 + m2);

	return 0;
}

int main() {
	// process:
	// getInput()
	// check if there was an error getting the input
	// calculateAtwoodAcceleration(mass[0] mass[1])
	// check if there was an error during the calculation
	// output acceleration to stdout

	int mass_inputs[N_MASS_INPUTS];
	if (getMassInputs(&mass_inputs, N_MASS_INPUTS)) {
		fputs("Failed to read inputs. Exiting.", stderr);
		return EXIT_FAILURE;
	}

	double acceleration;
	if (calculateAtwoodAcceleration(mass_inputs[0], mass_inputs[1], &acceleration)) {
		fputs("Despite input validation, calculation failed. One or both masses are negative. Exiting.", stderr);
		return EXIT_FAILURE;
	}

	printf("The acceleration of the weights in an Atwood machine with the given masses is:\n %.2f m/s\xFD\n", acceleration);
	printf("Press enter to exit...");
	getchar();
	return EXIT_SUCCESS;
}