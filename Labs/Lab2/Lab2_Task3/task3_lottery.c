/*
* 2025F COMP-166 Lab 2 - Thomas Boland - C0556991
*
* This program runs a simple lottery. The provided integer is compared with
* a random target from 00 to 99 and, depending on the number of matching
* digits, the payout amount is calculated.
*
* The program accepts one arguments from the command line:
* [guess]
* Where 0 <= guess <= 99
*
* If number of provided arguments is invalid, output a help description instead.
*
*/
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <errno.h>

#define JACKPOT_LARGE 10000
#define JACKPOT_MEDIUM 3000
#define JACKPOT_SMALL 1000

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
/// Compares the guess with the target using the lottery payout calculation rules and returns the payout amount.
/// </summary>
/// <param name="guess"></param>
/// <param name="target"></param>
/// <returns>Payout amount or -1 if guess not in range</returns>
int calculatePayout(int guess, int target) {
	// check validity of guess
	if (guess < 0 || guess > 99) {
		return -1;
	}

	short guess_tens = guess / 10;
	short guess_ones = guess % 10;

	short target_tens = target / 10;
	short target_ones = target % 10;

	if (guess == target) { // 2x correct and right order
		return JACKPOT_LARGE;
	}
	if (guess_ones == target_tens && guess_tens == target_ones) { // 2x correct and wrong order
		return JACKPOT_MEDIUM;
	}
	if (guess_ones == target_ones || guess_ones == target_tens ||
		guess_tens == target_ones || guess_tens == target_tens) { // 1x correct in any order
		return JACKPOT_SMALL;
	}
	return 0;
}

void outputHelp() {
	puts("Run a simple lottery\n");
	puts("Usage: Lab2_Task3 [guess]");
	puts("  [guess]  Number to guess. Integer from 0 to 99 inclusive.\n");
}

int main(int argc, char* argv[]) {
	// check for correct number of args
	// [program name] [guess]
	if (argc != 2) {
		outputHelp();
		return EXIT_FAILURE;
	}
	// convert input values:
	// if can't convert value to int:
	//   print error message and return fail
	int guess;
	if (parseInt(argv[1], &guess) != 0) {
		fputs("Unable to parse provided [guess].", stderr);
		return EXIT_FAILURE;
	}

	// generate target, split target digits, determine digits
	srand(time(NULL)); // milliseconds since system boot
	int target = rand() % 100;

	int payout = calculatePayout(guess, target);
	if (payout == -1) {
		// guess out of range.
		fputs("Provided guess must be between 0 and 99.", stderr);
		return EXIT_FAILURE;
	}

	printf("You guessed %02d and the target was %02d.\n", guess, target);

	if (payout == 0) {
		puts("You lost.");
	}
	else {
		printf("You won $%d!\n", payout);
	}

	return EXIT_SUCCESS;
}
