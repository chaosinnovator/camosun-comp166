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
#include <stdint.h>
#include <stdlib.h>
#include <errno.h>
#include <locale.h>
#include <math.h>
#include <string.h>
#include <sys/timeb.h>
#include <wchar.h>
#include <Windows.h>
#include <mmsystem.h>

typedef struct {
	char* guess_str;
	char* unknown_arg;
	bool help_flag;
} Arguments;

#define JACKPOT_LARGE 10000
#define JACKPOT_MEDIUM 3000
#define JACKPOT_SMALL 1000

typedef struct {
	unsigned short balance;
	unsigned short play_cost;
	unsigned short guess;
	unsigned char guess_digits[2];
	unsigned short target;
	unsigned char target_digits[2];
	unsigned long round_start_ms; // used for animation frames and to determine state
	bool is_playing;
	unsigned short round_payout;
} Game;

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
int splitDigits(int n, char* result_tens, char* result_ones) {
	// check validity of year number and month
	if (n < 0 || n > 99) {
		return -1;
	}

	*result_tens = n / 10;
	*result_ones = n % 10;

	return 0;
}

int calculatePayout(char guess_digits[2], char target_digits[2]) {
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

// declared here but defined later since it's not central to this lab
void displayFrame(wchar_t* output_buffer);
void play_generated_sound(bool);
const wchar_t wheel_digits[60][5];

void runGameLoop(Game game_state) {
	// set round_start time
	game_state.round_start_ms = GetTickCount64();
	// set up output rendering buffer with frame (frame shouldn't need to be re-rendered)
	wchar_t output_buffer[32*9] =
		L"    ╔══════════╦══════════╗    \n"
		L"    ║ $ 000000 ║ Bet 0000 ║    \n"
		L"╔═══╬══════════╩══════════╬═══╗\n"
		L"║ ♦ ║                     ║ ♠ ║\n"
		L"╠═══╣                     ╠═══╣\n"
		L"║ ♣ ║                     ║ ♥ ║\n"
		L"╠═══╣                     ╠═══╣\n"
		L"║ ♠ ║                     ║ ♦ ║\n"
		L"╚═══╩═════════════════════╩═══╝\n";
		// Enter guess or [Q]uit> ##
	// set up wheel digit array
	//    (made it a global const instead)
	// render frame to buffer. This should never have to be re-rendered
	// // ═ ║ ╔ ╗ ╚ ╝ ╠ ╣ ╦ ╩ ╬
	// ♠ ♥ ♣ ♦
	
	
	// render play_cost to buffer. this should never have to be re-rendered
	wchar_t num_buffer[16];
	swprintf_s(num_buffer, 16, L"%04d", game_state.play_cost > 9999 ? 9999 : game_state.play_cost);
	for (int i = 0; i < 4; i++) {
		output_buffer[(32 * 1 + 21) + i] = num_buffer[i];
	}
	// 
	// while playing
	unsigned long round_elapsed = 0;
	unsigned int wheel_offset_left = 0;
	unsigned int wheel_offset_right = 0;
	while (game_state.is_playing) {
		//   check round elapsed time
		round_elapsed = GetTickCount64() - game_state.round_start_ms;
		//   render frame decorations
		//   determine wheel display offsets, based on round elapsed time.
		//		<2000 ms, left wheel still spinning. speed = 10.0 digits per second. 6 rows per digit, gap each 6th row
		// play 2-wheel spinning sound.
		if (round_elapsed < 2000) {
			wheel_offset_left = (int)((game_state.target_digits[0] - (4.0 * (float)round_elapsed / 1000.0)) * 6) % 10;
		}
		else {
			wheel_offset_left = game_state.target_digits[0] * 6;
		}
		//		<4000 ms, right wheel still spinning
		// play 1-wheel spinning sound
		if (round_elapsed < 4000) {
			wheel_offset_right = (int)((game_state.target_digits[1] - (4.0 * (float)round_elapsed / 1000.0)) * 6) % 10;
		}
		else {
			wheel_offset_right = game_state.target_digits[1] * 6;
		}
		//   render wheels to the output buffer (only renders 5 rows)
		// left wheel
		for (int row = 0; row < 5; row++) {
			for (int col = 0; col < 5; col++) {
				output_buffer[(32 * (3 + row)) + (9 + col)] = wheel_digits[(wheel_offset_left + row) % 60][col];
			}
		}
		// right wheel
		for (int row = 0; row < 5; row++) {
			for (int col = 0; col < 5; col++) {
				output_buffer[(32 * (3 + row)) + (17 + col)] = wheel_digits[(wheel_offset_right + row) % 60][col];
			}
		}
		//   render guess.
		//   render balance. balance-payout if 500-4000ms, balance if >6000ms,
		//		animated decreasing from balance-payout+cost to balance-payout if <500ms,
		//      animated increasing from balance-payout to balance 4000-6000ms (if not win, still animate, but +0 doesn't do anything)
		//   if >4000ms render payout/win message. add to balance here? Play win or lose sound
		//   output buffer
		displayFrame(output_buffer);

		//   if >6000ms
		if (round_elapsed >= 6000) {
			printf("Enter guess or [Q]uit> ");
			// waiting for input
			char play_again_prompt[32];
			fgets(play_again_prompt, 32, stdin);
			//     if Q or q: quit
			if (play_again_prompt[0] == 'Q' || play_again_prompt[0] == 'q') {
				game_state.is_playing = false;
			}
			//     if try to continue with not enough money: "You're broke!" message & wait for input to quit.
			else if (game_state.balance < game_state.play_cost) {
				game_state.is_playing = false;
				printf("You don't have enough money to play. Press enter to exit...");
				getchar();
			}
			else {
				game_state.balance -= game_state.play_cost;
				if (play_again_prompt[0] != '\n') {
					//     if invalid input: don't reset and wait for input again.
					while (parseInt(play_again_prompt, &game_state.guess) != 0) {
						printf("Unable to parse. Try entering your guess again> ");
						fgets(play_again_prompt, 128, stdin);
					}
				}
				//     if new guess: split digits, generate new target, calculate payout, set new round_start time
				game_state.target = rand() % 100;
				splitDigits(game_state.target, &game_state.target_digits[0], &game_state.target_digits[1]);
				game_state.round_payout = calculatePayout(game_state.guess_digits, game_state.target_digits);
				game_state.round_start_ms = GetTickCount64();
			}
		}
		Sleep(50);
	}
}

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
	Game game_state = {
		.balance = 5000,		// Start player with $5000 balance.
		.play_cost = 490,		// Odds of a win are 42 in 100 (1/100 max + 1/100 med + 40/100 low). Mean payout should be about $530 per play.
				                //   Cost < 530 is an edge for the player, cost > 530 is an edge for the house.
		.guess = NULL,
		.guess_digits = NULL,
		.target = NULL,
		.target_digits = NULL,
		.round_start_ms = 0,
		.is_playing = true,
		.round_payout = 0,
	};
	if (parseInt(arg_struct.guess_str, &game_state.guess) != 0) {
		fputs("Unable to parse provided [guess].", stderr);
		return EXIT_FAILURE;
	}

	// split input digits. Do this first to also validate input range before
	// we bother to generate the target.
	if (splitDigits(game_state.guess, &game_state.guess_digits[0], &game_state.guess_digits[1])) {
		fputs("Provided guess must be between 0 and 99.", stderr);
		return EXIT_FAILURE;
	}

	// generate target
	srand(GetTickCount64()); // milliseconds since system boot
	game_state.target = rand() % 100;

	// split target digits
	splitDigits(game_state.target, &game_state.target_digits[0], &game_state.target_digits[1]); // no need to check success, we know target must be in range

	game_state.round_payout = calculatePayout(game_state.guess_digits, game_state.target_digits);
	game_state.balance += game_state.round_payout;

	runGameLoop(game_state);

	do {

		// output
		printf("You guessed %02d and the winning number was %02d.\n", game_state.guess, game_state.target);
		if (game_state.round_payout == 0) {
			puts("Maybe next time!");
		}
		else {
			printf("You won $%d!\n", game_state.round_payout);
		}

		play_generated_sound(game_state.round_payout > 0);
		Sleep(2000); // Sound plays asynchronously. Wait until sound has played before prompting for input.

		//system("cls"); // clear screen

		printf("Balance: $%d. Enter number to play again (blank = same guess) for $%d or Q to quit> ", game_state.balance, game_state.play_cost);
		char play_again_prompt[128];
		fgets(play_again_prompt, 128, stdin);
		if (play_again_prompt[0] == 'Q' || play_again_prompt[0] == 'q') {
			game_state.is_playing = false;
		}
		else if (game_state.balance < game_state.play_cost) {
			game_state.is_playing = false;
			printf("You don't have enough money to play. Press enter to exit...");
			getchar();
		}
		else {
			game_state.balance -= game_state.play_cost;
			if (play_again_prompt[0] != '\n') {
				while (parseInt(play_again_prompt, &game_state.guess) != 0) {
					printf("Unable to parse. Try entering your guess again> ");
					fgets(play_again_prompt, 128, stdin);
				}
			}
			// generate new target
			game_state.target = rand() % 100;
			splitDigits(game_state.target, &game_state.target_digits[0], &game_state.target_digits[1]);
			game_state.round_payout = calculatePayout(game_state.guess_digits, game_state.target_digits);
		}
	} while (game_state.is_playing);

	return EXIT_SUCCESS;
}


// Helper to display a frame of the game output
void displayFrame(wchar_t* output_buffer) {
	system("cls"); // clear screen. probably a better way to do this with Win32 Console API calls, but I didn't want to figure that out.
	HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
	WriteConsoleW(hConsole, output_buffer, 32*9, NULL, NULL); // couldn't get wprint to work
}


// Digits the get rendered as the slot wheels, like:
//   █ 
// ███ 
//   █ 
//   █ 
// █████
const wchar_t wheel_digits[60][5] = {
	{L'█', L'█', L'█', L'█', L'█'}, // 0
	{L'█', L' ', L' ', L' ', L'█'},
	{L'█', L' ', L' ', L' ', L'█'},
	{L'█', L' ', L' ', L' ', L'█'},
	{L'█', L'█', L'█', L'█', L'█'},
	{L' ', L' ', L' ', L' ', L' '},
	{L' ', L' ', L'█', L' ', L' '}, // 1
	{L'█', L'█', L'█', L' ', L' '},
	{L' ', L' ', L'█', L' ', L' '},
	{L' ', L' ', L'█', L' ', L' '},
	{L'█', L'█', L'█', L'█', L'█'},
	{L' ', L' ', L' ', L' ', L' '},
	{L'█', L'█', L'█', L'█', L'█'}, // 2
	{L' ', L' ', L' ', L' ', L'█'},
	{L'█', L'█', L'█', L'█', L'█'},
	{L'█', L' ', L' ', L' ', L' '},
	{L'█', L'█', L'█', L'█', L'█'},
	{L' ', L' ', L' ', L' ', L' '},
	{L'█', L'█', L'█', L'█', L'█'}, // 3
	{L' ', L' ', L' ', L' ', L'█'},
	{L' ', L'█', L'█', L'█', L'█'},
	{L' ', L' ', L' ', L' ', L'█'},
	{L'█', L'█', L'█', L'█', L'█'},
	{L' ', L' ', L' ', L' ', L' '},
	{L' ', L'█', L' ', L' ', L'█'}, // 4
	{L'█', L' ', L' ', L' ', L'█'},
	{L'█', L'█', L'█', L'█', L'█'},
	{L' ', L' ', L' ', L' ', L'█'},
	{L' ', L' ', L' ', L' ', L'█'},
	{L' ', L' ', L' ', L' ', L' '},
	{L'█', L'█', L'█', L'█', L'█'}, // 5
	{L'█', L' ', L' ', L' ', L' '},
	{L'█', L'█', L'█', L'█', L'█'},
	{L' ', L' ', L' ', L' ', L'█'},
	{L'█', L'█', L'█', L'█', L'█'},
	{L' ', L' ', L' ', L' ', L' '},
	{L'█', L'█', L'█', L'█', L'█'}, // 6
	{L'█', L' ', L' ', L' ', L' '},
	{L'█', L'█', L'█', L'█', L'█'},
	{L'█', L' ', L' ', L' ', L'█'},
	{L'█', L'█', L'█', L'█', L'█'},
	{L' ', L' ', L' ', L' ', L' '},
	{L'█', L'█', L'█', L'█', L'█'}, // 7
	{L' ', L' ', L' ', L' ', L'█'},
	{L' ', L' ', L'█', L'█', L' '},
	{L' ', L' ', L'█', L' ', L' '},
	{L' ', L' ', L'█', L' ', L' '},
	{L' ', L' ', L' ', L' ', L' '},
	{L'█', L'█', L'█', L'█', L'█'}, // 8
	{L'█', L' ', L' ', L' ', L'█'},
	{L'█', L'█', L'█', L'█', L'█'},
	{L'█', L' ', L' ', L' ', L'█'},
	{L'█', L'█', L'█', L'█', L'█'},
	{L' ', L' ', L' ', L' ', L' '},
	{L'█', L'█', L'█', L'█', L'█'}, // 9
	{L'█', L' ', L' ', L' ', L'█'},
	{L'█', L'█', L'█', L'█', L'█'},
	{L' ', L' ', L' ', L' ', L'█'},
	{L'█', L'█', L'█', L'█', L'█'},
	{L' ', L' ', L' ', L' ', L' '},
};

/****************************************************************
*               Sound Generator (Windows only)                  *
* I needed to find samples for most of this as a reference,     *
* but needed to understand how it works to change the melodies, *
* and change the tone to square wave instead of pure sine wave. *
* Comments are added by me as I figured out how it works.       *
*****************************************************************/

#pragma comment(lib, "winmm.lib")

#define SAMPLE_RATE 44100
#define DURATION 2.0
#define AMPLITUDE 10000
#ifndef M_PI
	#define M_PI 3.141592653589793 // Visual studio doesn't define M_PI in this implementation of math.h
#endif // !M_PI


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

void generateSquareTone(int16_t* sample_buffer, double freq, double duration, int offset) {
	int count = (int)(SAMPLE_RATE * duration); // how many samples/"frames" to run this tone for?
	for (int i = 0; i < count; i++) {
		double t = (double)i / SAMPLE_RATE;
		sample_buffer[offset + i] = (sin(2 * M_PI * freq * t) > 0 ? AMPLITUDE : -AMPLITUDE); // square wave as a function of time
	}
}

void play_generated_sound(bool win) {
	int totalSamples = (int)(SAMPLE_RATE * DURATION);
	int dataSize = totalSamples * sizeof(int16_t);
	int totalSize = sizeof(WAVHeader) + dataSize;

	uint8_t* buffer = (uint8_t*)malloc(totalSize); // pointer block of memory to store the generated sound buffer
	int16_t* sample_buffer = (int16_t*)(buffer + sizeof(WAVHeader)); // pointer to an offset in the buffer to start storing sound data (after header)

	writeWAVHeader(buffer, dataSize);

	// Winning melody: C6 G5 F5 C5 x6
	double win_tones[24] = {
			1046.50, 783.99, 698.46, 523.25, /**/ 1046.50, 783.99, 698.46, 523.25, /**/ 1046.50, 783.99, 698.46, 523.25,
			1046.50, 783.99, 698.46, 523.25, /**/ 1046.50, 783.99, 698.46, 523.25, /**/ 1046.50, 783.99, 698.46, 523.25
	};

	// Losing melody: B5 G4b E4b C4 x6
	double lose_tones[4] = {
		493.88, 369.99, 311.13, 261.63
	};

	if (win) {
		int samplesPerTone = totalSamples / 24;
		for (int i = 0; i < 24; i++) {
			generateSquareTone(sample_buffer, win_tones[i], DURATION / 24, i * samplesPerTone);
		}
	}
	else {
		int samplesPerTone = totalSamples / 4;
		for (int i = 0; i < 4; i++) {
			generateSquareTone(sample_buffer, lose_tones[i], DURATION / 4, i * samplesPerTone);
		}
	}

	// Play directly from memory
	PlaySound((LPCSTR)buffer, NULL, SND_MEMORY | SND_ASYNC); // SND_ASYNC flag plays sound asynchronously

	free(buffer);
}
