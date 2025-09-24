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
* If provided arguments are invalid or any argument is /?, output a help description instead.
*
*/
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <sys/timeb.h>
#include <Windows.h>

typedef struct {
	char* guess_str;
	char* unknown_arg;
	bool help_flag;
} Arguments;

#define JACKPOT_LARGE 10000
#define JACKPOT_MEDIUM 3000
#define JACKPOT_SMALL 1000

/// <summary>
/// Check that one argument were provided. Additionally, check if help was requested or if
/// invalid arguments were provided
/// <param name="*arg_struct">Where to put result</param>
/// <returns>Returns 0 if successful, non-zero if not successful.</returns>
int parseArgs(int argc, char* argv[], Arguments* arg_struct) {
	// check each provided arg.
	// if /?, set help_flag to true and return 0
	// if first char is not / and guess_str hasn't been set yet, set it and continue
	// if there are any other arguments, set unknown_arg, set help_flag to true, return -1
	// if guess_str was not set, set help_flag to true. This covers the case if no arguments provided.

	for (int i = 1; i < argc; i++) // first arg is the program name. skip it.
	{
		if (strcmp(argv[i], "/?") == 0) {
			(*arg_struct).help_flag = true;
			return 0;
		}
		if (argv[i][0] != '/' && (*arg_struct).guess_str == NULL) {
			(*arg_struct).guess_str = argv[i];
			continue;
		}
		(*arg_struct).unknown_arg = argv[i];
		(*arg_struct).help_flag = true;
		return -1;
	}
	// only need to check month_str to determine if both were set, since they get set in order
	// if month_str is set, year_str must have also been set already.
	if ((*arg_struct).guess_str == NULL) {
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
/// Splits a positive integer with up to two digits into the tens digit and the ones digit
/// </summary>
/// <param name="n">integer to split</param>
/// <param name="*result_tens">Where to put tens</param>
/// <param name="*result_ones">Where to put ones</param>
/// <returns>Returns 0 if successful, non-zero if not successful.</returns>
int splitDigits(int n, int* result_tens, int* result_ones) {
	// check validity of year number and month
	if (n < 0 || n > 99) {
		return -1;
	}

	*result_tens = n / 10;
	*result_ones = n % 10;

	return 0;
}

int calculatePayout(int guess_digits[2], int target_digits[2]) {
	if (guess_digits[0] == target_digits[0] && guess_digits[1] == target_digits[1]) { // 2x correct and right order
		return JACKPOT_LARGE;
	}
	if (guess_digits[0] == target_digits[1] && guess_digits[1] == target_digits[0]) { // 2x correct and wrong order
		return JACKPOT_MEDIUM;
	}
	if (guess_digits[0] == target_digits[0] || guess_digits[0] == target_digits[1] ||
		guess_digits[1] == target_digits[0] || guess_digits[1] == target_digits[1]) { // 1x correct in any order
		return JACKPOT_SMALL;
	}
	return 0;
}

void outputHelp() {
	puts("Run a simple lottery\n");
	puts("Usage: Lab2_Task3 [guess]");
	puts("  [guess]  Number to guess. Integer from 0 to 99 inclusive.");
	puts("  /?       Display this help page.\n");
}

void play_generated_sound(bool);

int main(int argc, char* argv[]) {
	// parse input argument strings. returns -1 (and sets help flag) if error
	// if error:
	//   Unknown argument "[arg]"
	Arguments arg_struct = {
		NULL, // guess_str
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
	int guess;
	if (parseInt(arg_struct.guess_str, &guess) != 0) {
		fputs("Unable to parse provided [guess].", stderr);
		return EXIT_FAILURE;
	}

	// if this was played continuously, this would be the top of the game loop. Start player with $5000 balance.
	int balance = 5000;
	// Odds of a win are 42 in 100 (1/100 max + 1/100 med + 40/100 low). Mean payout should be about $530 per play.
	// Cost less than 530 is an edge for the player, cost more than 530 is an edge for the house.
	int play_cost = 490;
	bool playing = true;
	while (playing) {
		// split input digits. Do this first to also validate input range before
		// we bother to generate the target.
		int guess_digits[2];
		if (splitDigits(guess, &guess_digits[0], &guess_digits[1])) {
			fputs("Provided guess must be between 0 and 99.", stderr);
			return EXIT_FAILURE;
		}

		// generate target
		srand(GetTickCount64());
		int target = rand() % 100;

		// split target digits
		int target_digits[2];
		splitDigits(target, &target_digits[0], &target_digits[1]); // no need to check success, we know target must be in range

		int payout = calculatePayout(guess_digits, target_digits);
		balance += payout;

		// output
		printf("You guessed %02d and the winning number was %02d.\n", guess, target);
		if (payout == 0) {
			puts("Maybe next time!");
		}
		else {
			printf("You won $%d!\n", payout);
		}

		play_generated_sound(payout > 0);

		printf("\nBalance: $%d. Enter number to play again (blank = same guess) for $%d or Q to quit> ", balance, play_cost);
		char play_again_prompt[128];
		fgets(play_again_prompt, 128, stdin);
		if (play_again_prompt[0] == 'Q' || play_again_prompt[0] == 'q') {
			playing = false;
		}
		else if (balance < play_cost) {
			playing = false;
			printf("You don't have enough money to play. Press enter to exit...");
			getchar();
		}
		else {
			balance -= play_cost;
			if (play_again_prompt[0] != '\n') {
				while (parseInt(play_again_prompt, &guess) != 0) {
					printf("Unable to parse. Try entering your guess again> ");
					fgets(play_again_prompt, 128, stdin);
				}
			}
		}
	}

	return EXIT_SUCCESS;
}


// Sound generator

#include <windows.h>
#include <mmsystem.h>
#include <stdint.h>
#include <math.h>
#include <stdio.h>

#pragma comment(lib, "winmm.lib")

#define SAMPLE_RATE 44100
#define DURATION 2.0
#define AMPLITUDE 10000

// WAV header structure
typedef struct {
	char chunkID[4];      // "RIFF"
	uint32_t chunkSize;
	char format[4];       // "WAVE"
	char subchunk1ID[4];  // "fmt "
	uint32_t subchunk1Size;
	uint16_t audioFormat;
	uint16_t numChannels;
	uint32_t sampleRate;
	uint32_t byteRate;
	uint16_t blockAlign;
	uint16_t bitsPerSample;
	char subchunk2ID[4];  // "data"
	uint32_t subchunk2Size;
} WAVHeader;

void writeWAVHeader(uint8_t* buffer, int dataSize) {
	WAVHeader header = {
		{'R','I','F','F'},
		36 + dataSize,
		{'W','A','V','E'},
		{'f','m','t',' '},
		16,
		1, // PCM
		1, // mono
		SAMPLE_RATE,
		SAMPLE_RATE * 2,
		2,
		16,
		{'d','a','t','a'},
		dataSize
	};
	memcpy(buffer, &header, sizeof(header));
}

void generateTone(int16_t* samples, double freq, double duration, int offset) {
	int count = (int)(SAMPLE_RATE * duration);
	for (int i = 0; i < count; i++) {
		double t = (double)i / SAMPLE_RATE;
		samples[offset + i] = (sin(2 * 3.1415 * freq * t) > 0 ? AMPLITUDE : -AMPLITUDE); // square wave
	}
}

void play_generated_sound(bool win) {
	int totalSamples = (int)(SAMPLE_RATE * DURATION);
	int dataSize = totalSamples * sizeof(int16_t);
	int totalSize = sizeof(WAVHeader) + dataSize;

	uint8_t* buffer = (uint8_t*)malloc(totalSize);
	int16_t* samples = (int16_t*)(buffer + sizeof(WAVHeader));

	writeWAVHeader(buffer, dataSize);

	// Win melody C6 G5 F5 C5 x6
	double win_tones[24] = {
			1046.50, 783.99, 698.46, 523.25, 1046.50, 783.99, 698.46, 523.25, 1046.50, 783.99, 698.46, 523.25,
			1046.50, 783.99, 698.46, 523.25, 1046.50, 783.99, 698.46, 523.25, 1046.50, 783.99, 698.46, 523.25
	};
	
	// Lose melody C5 G4 E4 C4 x6
	double lose_tones[4] = {
		523.25, 392.00, 329.63, 261.63
	};
	
	if (win) {
		int samplesPerTone = totalSamples / 24;
		for (int i = 0; i < 24; i++) {
			generateTone(samples, win_tones[i], DURATION / 24, i * samplesPerTone);
		}
	}
	else {
		int samplesPerTone = totalSamples / 4;
		for (int i = 0; i < 4; i++) {
			generateTone(samples, lose_tones[i], DURATION / 4, i * samplesPerTone);
		}
	}

	// Play directly from memory
	PlaySound((LPCSTR)buffer, NULL, SND_MEMORY | SND_ASYNC);

	Sleep((int)(DURATION * 1000));  // Let it finish
	free(buffer);
}
