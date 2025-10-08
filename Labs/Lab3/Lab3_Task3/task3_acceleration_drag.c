/*
* 2025F COMP-166 Lab 3 - Thomas Boland - C0556991
*
* This program prints a right triangle, isosceles triangle, or diamond (inverted or not), using a specified character.
*
*/

#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <errno.h>

#define MAX_ITERATIONS 2000
#define GRAVITY_ACCELERATION -9.8

/**
* @brief Get a float input from stdin. Force the user to retry until input is valid.
* @param *input_value Where to put successful input
* @param *prompt String to display for each prompt
* @return Returns 0 if successful, non-zero if failed
*/
int getFloatInput(float* input_value, char* prompt) {
	char input_buf[128];
	float input;
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
		input = strtof(input_buf, &first_unconverted_character);

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
			printf("Input out of range. Please try again.\n");
			continue;
		}

		// success
		(*input_value) = input;
		break;
	}
	return 0;
}

int main() {
	// get and validate inputs (all inputs must be positive; getFloatInput enforces positive):
	//  float initial drop height (meters)
	//  float object mass (kg)
	//  float cross-sectional area (m^2)
	//  float drag coefficient
	//  float atmospheric density (kg/m^3)
	//  float simulation_time_step (seconds)
	float position_y;
	float mass;
	float area;
	float drag_coefficient;
	float density_atmosphere;
	float sim_time_step;
	if (getFloatInput(&position_y, "Enter initial drop height (meters): ") != 0 ||
		getFloatInput(&mass, "Enter object mass (kg): ") != 0 || 
		getFloatInput(&area, "Enter cross-sectional (m^2): ") != 0 ||
		getFloatInput(&drag_coefficient, "Enter drag coefficient: ") !=  0 ||
		getFloatInput(&density_atmosphere, "Enter atmospheric density (kg/m^3): ") != 0 ||
		getFloatInput(&sim_time_step, "Enter the simulation time step (seconds): ")) {
		fputs("Failed to get input.", stderr);
		return EXIT_FAILURE;
	}

	// print data headers
	printf("time (s)\theight (m)\tvelocity (m/s)\n");

	// sim_time, velocity = 0
	float sim_time = 0.0;
	float velocity_y = 0.0;
	float force_drag = 0.0;
	float acceleration_y = 0.0;
	while (sim_time < (sim_time_step * MAX_ITERATIONS)) {
		// print data row
		// if height <= 0, collided with ground. height = 0, velocity = 0, stop simulation.
		// time		height		velocity
		printf("%8.3f\t%8.3f\t%8.3f\n", sim_time, (position_y > 0.0 ? position_y : 0.0), (position_y > 0.0 ? velocity_y : 0.0));
		if (position_y <= 0) {
			break;
		}

		// calculate drag force
		// Fd =	(C𝜌Av^2)/2
		force_drag = (drag_coefficient * density_atmosphere * area * velocity_y * velocity_y) / 2;
		// calculate net force -> acceleration
		acceleration_y = (mass * GRAVITY_ACCELERATION + force_drag) / mass;
		
		// calculate new velocity
		velocity_y += acceleration_y * sim_time_step;
		// calculate displacement/new height
		position_y += velocity_y * sim_time_step;
		// increment sim_time
		sim_time += sim_time_step;
	}
}