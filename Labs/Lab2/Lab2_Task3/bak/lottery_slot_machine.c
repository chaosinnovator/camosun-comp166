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

#define JACKPOT_LARGE 10000
#define JACKPOT_MEDIUM 3000
#define JACKPOT_SMALL 1000

typedef struct {
	char* guess_str;
	char* unknown_arg;
	bool help_flag;
} Arguments;

typedef struct {
	unsigned short balance;
	unsigned short play_cost;
	unsigned short guess;
	unsigned short guess_digits[2];
	unsigned short target;
	unsigned short target_digits[2];
	unsigned long round_start_ms; // used for animation frames and to determine state
	bool is_playing;
	unsigned short round_payout;
} Game;

// declared here but defined later since these are for extra functionality.
void runGameLoop(Game game_state);
void displayFrame(wchar_t* output_buffer);
void playWinSound(float duration);
void playLoseSound(float duration);
const wchar_t wheel_digits[60][5];

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
int splitDigits(int n, short* result_tens, short* result_ones) {
	// check validity of guess
	if (n < 0 || n > 99) {
		return -1;
	}

	*result_tens = n / 10;
	*result_ones = n % 10;

	return 0;
}

/// <summary>
/// Compares the guess with the target using the lottery payout calculation rules and returns the payout amount.
/// </summary>
/// <param name="*guess_digits">Array of two integers</param>
/// <param name="*target_digits">Array of two integers</param>
/// <returns>Payout amount</returns>
int calculatePayout(short* guess_digits, short* target_digits) {
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

	// Initialize game state (
	Game game_state = {
		.balance = 5000,		// Start player with $5000 balance.
		.play_cost = 490,		// Odds of any win are 42 in 100 (1/100 max + 1/100 med + 40/100 low). Mean payout should be about $530 per play (napkin math).
		                        //   Cost < 530 is an edge for the player, cost > 530 is an edge for the house.
		.guess = NULL,
		.guess_digits = NULL,
		.target = NULL,
		.target_digits = NULL,
		.round_start_ms = 0,
		.is_playing = true,
		.round_payout = 0,
	};

	// convert input values:
	// if can't convert value to int:
	//   print error message and return fail
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

	// generate target, split target digits, determine digits
	srand(GetTickCount64()); // milliseconds since system boot
	game_state.target = rand() % 100;
	splitDigits(game_state.target, &game_state.target_digits[0], &game_state.target_digits[1]); // no need to check success, we know target must be in range

	game_state.round_payout = calculatePayout(game_state.guess_digits, game_state.target_digits);

	/**********************************************************************************
	* Regular lab would end here with a printf to display the payout amount.          *
	* Instead, we'll enter a game loop to display a "slot machine" to present         *
	* the result in a more interactive way with animation, sound, and replay options. *
	**********************************************************************************/

	//printf("You won $%d!", game_state.round_payout);

	game_state.balance += game_state.round_payout;
	system("cls"); // clear screen. probably a better way to do this with Win32 Console API calls, but I didn't bother to figure that out.
	runGameLoop(game_state);

	return EXIT_SUCCESS;
}


/****************************************************************
*              Functions for extra functionality                *
****************************************************************/

#define FRAME_COLUMNS 32
#define FRAME_ROWS 9
#define DIGIT_WIDTH 5
#define DIGIT_HEIGHT 6
#define N_DECORATIONS 8
#define DECORATION_ANIMATION_INTERVAL 180
#define LEFT_WHEEL_SPIN_DURATION 5000
#define RIGHT_WHEEL_SPIN_DURATION 7000 // Must be >= LEFT_WHEEL_SPIN_DURATION
#define PAYOUT_ANIMATION_DURATION 2000
#define COST_ANIMATION_DURATION 1000
#define LEFT_WHEEL_ACCELERATION 1.0
#define RIGHT_WHEEL_ACCELERATION 0.6
#define FPS_TARGET 30

void runGameLoop(Game game_state) {
	// set round_start time
	game_state.round_start_ms = GetTickCount64();
	// set up output rendering buffer with frame (frame shouldn't need to be re-rendered)
	// ═ ║ ╔ ╗ ╚ ╝ ╠ ╣ ╦ ╩ ╬ ♠ ♥ ♣ ♦   <-- characters used (here for easy copy/pasting).
	// Unicode characters, so from here onwards all output strings use wchars.
	wchar_t output_buffer[FRAME_COLUMNS * FRAME_ROWS] =
		L"╔═══╦══════════╦══════════╦═══╗\n"
		L"║ ♠ ║ $ ------ ║ Bet ---- ║ ♣ ║\n"
		L"╠═══╬══════════╩══════════╬═══╣\n"
		L"║ ♦ ║                     ║ ♠ ║\n"
		L"╠═══╣                     ╠═══╣\n"
		L"║ ♣ ║  ►               ◄  ║ ♥ ║\n"
		L"╠═══╣                     ╠═══╣\n"
		L"║ ♠ ║                     ║ ♦ ║\n"
		L"╚═══╩═════════════════════╩═══╝\n";
	   // Enter guess or [Q]uit> ##
	// set up wheel digit array (made it a global const instead)

	wchar_t decoration_chars[4] = {L'♠', L'♥', L'♣', L'♦'};
	// array of int[2] representing {row, col} position of each decoration character
	unsigned short deco_char_positions[N_DECORATIONS][2] = {
		{1, 2},
		{3, 2},
		{5, 2},
		{7, 2},
		{1, 28},
		{3, 28},
		{5, 28},
		{7, 28},
	};

	// render play_cost to buffer. this should never have to be re-rendered
	wchar_t num_buffer[16]; // buffer to printf formatted strings to before copying to output_buffer
	swprintf_s(num_buffer, 16, L"%04d", game_state.play_cost > 9999 ? 9999 : game_state.play_cost);
	for (int i = 0; i < 4; i++) {
		output_buffer[(FRAME_COLUMNS * 1 + 21) + i] = num_buffer[i];
	}

	// enter game loop
	unsigned long round_elapsed = 0;
	unsigned long frame_start = 0;
	unsigned int wheel_offset_left = 0;
	unsigned int wheel_offset_right = 0;
	unsigned int rendered_balance = 0;
	bool played_end_sound = false;
	unsigned long last_decoration_animation = 0;
	while (game_state.is_playing) {
		// check round elapsed time
		frame_start = GetTickCount64();
		round_elapsed = frame_start - game_state.round_start_ms;
		// render frame decorations
		if (round_elapsed - last_decoration_animation > DECORATION_ANIMATION_INTERVAL) {
			last_decoration_animation = round_elapsed;
			for (int i = 0; i < N_DECORATIONS; i++) {
				output_buffer[(FRAME_COLUMNS * deco_char_positions[i][0]) + deco_char_positions[i][1]] = decoration_chars[rand() % 4];
			}
		}
		
		// Determine digit wheel positions
		// Left wheel spins for 2000ms
		// play 2-wheel spinning sound.
		if (round_elapsed < LEFT_WHEEL_SPIN_DURATION) {
			// digit# - accel*t*t --> convert to digit coordinate based on DIGIT_HEIGHT
			wheel_offset_left = (int)((game_state.target_digits[0] - LEFT_WHEEL_ACCELERATION * ((LEFT_WHEEL_SPIN_DURATION - (float)round_elapsed) / 1000.0) * ((LEFT_WHEEL_SPIN_DURATION - (float)round_elapsed) / 1000.0)) * DIGIT_HEIGHT);
		}
		else {
			wheel_offset_left = game_state.target_digits[0] * DIGIT_HEIGHT;
		}
		// Right wheel spins for 4000ms
		// play 1-wheel spinning sound
		if (round_elapsed < RIGHT_WHEEL_SPIN_DURATION) {
			// digit# - accel*t*t --> convert to digit coordinate based on DIGIT_HEIGHT
			wheel_offset_right = (int)((game_state.target_digits[1] - RIGHT_WHEEL_ACCELERATION * ((RIGHT_WHEEL_SPIN_DURATION - (float)round_elapsed) / 1000.0) * ((RIGHT_WHEEL_SPIN_DURATION - (float)round_elapsed) / 1000.0)) * DIGIT_HEIGHT);
		}
		else {
			wheel_offset_right = game_state.target_digits[1] * DIGIT_HEIGHT;
		}

		// Render wheels to the output buffer (only renders 5 rows since that is the available space inside the frame design)
		// left wheel
		for (int row = 0; row < 5; row++) {
			for (int col = 0; col < DIGIT_WIDTH; col++) {
				output_buffer[(FRAME_COLUMNS * (3 + row)) + (9 + col)] = wheel_digits[(wheel_offset_left + row) % (DIGIT_HEIGHT * 10)][col];
			}
		}
		// right wheel
		for (int row = 0; row < 5; row++) {
			for (int col = 0; col < DIGIT_WIDTH; col++) {
				output_buffer[(FRAME_COLUMNS * (3 + row)) + (17 + col)] = wheel_digits[(wheel_offset_right + row) % (DIGIT_HEIGHT * 10)][col];
			}
		}

		// Render guess digits
		swprintf_s(num_buffer, 16, L"%02d", game_state.guess);
		output_buffer[(FRAME_COLUMNS * 5) + 6] = num_buffer[0];
		output_buffer[(FRAME_COLUMNS * 5) + 24] = num_buffer[1];

		// Render balance.
		// animate decreasing from balance-payout+cost to balance-payout if <500ms,
		if (round_elapsed < COST_ANIMATION_DURATION) {
			rendered_balance = (int)((game_state.balance - game_state.round_payout) + (float)game_state.play_cost * (1.0 - (float)round_elapsed / COST_ANIMATION_DURATION));
		}
		else if (round_elapsed < RIGHT_WHEEL_SPIN_DURATION) {
			rendered_balance = game_state.balance - game_state.round_payout;
		}
		// animate increasing from balance-payout to balance 4000-6000ms (if no payout still animating this is fine because -0*x doesn't do anything)
		else if (round_elapsed < RIGHT_WHEEL_SPIN_DURATION + PAYOUT_ANIMATION_DURATION) {
			rendered_balance = (int)(game_state.balance - (float)game_state.round_payout * (RIGHT_WHEEL_SPIN_DURATION + PAYOUT_ANIMATION_DURATION - (float)round_elapsed) / PAYOUT_ANIMATION_DURATION);
		}
		else {
			rendered_balance = game_state.balance;
		}
		swprintf_s(num_buffer, 16, L"%06d", rendered_balance > 999999 ? 999999 : rendered_balance);
		for (int i = 0; i < 6; i++) {
			output_buffer[(FRAME_COLUMNS * 1 + 8) + i] = num_buffer[i];
		}

		// If >4000ms render flashing payout/win message. If >6000ms not flashing. Play win/lose sound.
		if (round_elapsed >= RIGHT_WHEEL_SPIN_DURATION) {
			if (round_elapsed % 700 < 400 || round_elapsed >= RIGHT_WHEEL_SPIN_DURATION + PAYOUT_ANIMATION_DURATION) {
				if (game_state.round_payout > 0) {
					swprintf_s(num_buffer, 16, L"You won $%5d!", game_state.round_payout);
					//play win sound
					if (!played_end_sound) {
						played_end_sound = true;
						playWinSound(PAYOUT_ANIMATION_DURATION / 1000.0);
					}
				}
				else {
					swprintf_s(num_buffer, 16, L"═══You lost!═══");
					//play lose sound
					if (!played_end_sound) {
						played_end_sound = true;
						playLoseSound(PAYOUT_ANIMATION_DURATION / 1000.0);
					}
				}
				for (int i = 0; i < 15; i++) {
					output_buffer[(FRAME_COLUMNS * 8) + 8 + i] = num_buffer[i];
				}
			}
			else {
				for (int i = 0; i < 15; i++) {
					output_buffer[(FRAME_COLUMNS * 8) + 8 + i] = L'═';
				}
			}
		}

		// Render output buffer to console
		displayFrame(output_buffer);

		// Display menu/prompt if >6000ms
		if (round_elapsed >= RIGHT_WHEEL_SPIN_DURATION + PAYOUT_ANIMATION_DURATION) {
			printf("Enter guess or [Q]uit> ");
			// waiting for input
			char play_again_prompt[32];
			fgets(play_again_prompt, 32, stdin);
			// If Q or q: quit
			if (play_again_prompt[0] == 'Q' || play_again_prompt[0] == 'q') {
				game_state.is_playing = false;
			}
			// If try to continue with not enough money: "You're broke!" message & wait for input to quit.
			else if (game_state.balance < game_state.play_cost) {
				game_state.is_playing = false;
				printf("You don't have enough money to play. Press enter to exit...");
				getchar();
			}
			else {
				// If invalid input: don't reset but do another iteration of loop and wait for input again.
				// No input = play again with same guess.
				if (play_again_prompt[0] != '\n' && parseInt(play_again_prompt, &game_state.guess) != 0 || game_state.guess > 99) {
					puts("Invalid input. Try again.");
					Sleep(1000);
					system("cls"); // clear screen. probably a better way to do this with Win32 Console API calls, but I didn't bother to figure that out.
					continue;
				}

				// Starting a new round, reset: deduct cost, split digits, generate new target, calculate payout, set new round_start time
				game_state.target = rand() % 100;
				splitDigits(game_state.target, &game_state.target_digits[0], &game_state.target_digits[1]);
				splitDigits(game_state.guess, &game_state.guess_digits[0], &game_state.guess_digits[1]);

				game_state.balance -= game_state.play_cost;
				game_state.round_payout = calculatePayout(game_state.guess_digits, game_state.target_digits);
				game_state.balance += game_state.round_payout;

				game_state.round_start_ms = GetTickCount64();
				played_end_sound = false;

				// fix bottom of frame
				for (int i = 0; i < 15; i++) {
					output_buffer[(FRAME_COLUMNS * 8) + 8 + i] = L'═';
				}
			}
		}
		else {
			// displayFrame doesn't erase the input line(s). Do that here if not showing the menu.
			printf("                                                                \n");
			
			// limit framerate. Subtract frame duration from this for proper FPS rate matching
			unsigned long frame_duration = (GetTickCount64() - frame_start);
			if (frame_duration < 1000 / FPS_TARGET) {
				Sleep(1000 / FPS_TARGET - frame_duration);
			}
		}
	}
}

// Helper to display a frame of the game output
void displayFrame(wchar_t* output_buffer) {
	COORD coord;
	coord.X = 0;
	coord.Y = 0;
	HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
	SetConsoleCursorPosition(hConsole, coord); // Win32 Console API call to move the cursor to top-left
	WriteConsoleW(hConsole, output_buffer, FRAME_COLUMNS * FRAME_ROWS, NULL, NULL); // couldn't get wprint to work. Win32 Console API call, bypasses printf and outputs UTF-16 directly.
}


// Digits that get rendered as the slot machine wheels, like:
//   █ 
// ███ 
//   █ 
//   █ 
// █████
// 
// ^ row of ' ' between each digit.
const wchar_t wheel_digits[DIGIT_HEIGHT * 10][DIGIT_WIDTH] = {
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
	{L'█', L' ', L' ', L' ', L'█'}, // 4
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
	{L' ', L' ', L' ', L'█', L' '},
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
****************************************************************/

#pragma comment(lib, "winmm.lib")

#define SAMPLE_RATE 44100 // samples per second
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

void writeWAVHeader(uint8_t* buffer, int dataSize, int n_channels, int sample_rate) {
	// See https://en.wikipedia.org/wiki/WAV#WAV_file_header
	WAVHeader header = {
		"RIFF", // FileTypeBlocID = "RIFF"
		36 + dataSize, // Remaining file size (total size - 8bytes). Header is 44 bytes, -8 = 36
		"WAVE", // FileFormatID = "WAVE"
		// Format chunk
		"fmt ", // FormatBlocID = "fmt "
		16, // BlocSize Remaining chunk size (16 bytes) = chunk size - 8 bytes
		1, // AudioFormat 1=PCM integer, 2
		n_channels, // NumChannels. 1=mono
		sample_rate, // Frequency
		sample_rate * 2 * n_channels, // BytePerSec = Frequency*BytePerBloc
		2 * n_channels, // BytePerBlock = NbrChannels*BitsPerSample/8
		16, // BitsPerSample. int16_t = 16 bits
		// Data chunk
		"data", // DataBlocID = "data"
		dataSize // Sample data size
	};
	memcpy(buffer, &header, sizeof(header));
}

void generateSquareTone(int16_t* sample_buffer, double freq, double duration, int offset, int sample_rate, int amplitude) {
	int count = (int)(sample_rate * duration); // how many samples/"frames" to run this tone for?
	for (int i = 0; i < count; i++) {
		double t = (double)i / sample_rate;
		sample_buffer[offset + i] = (sin(2 * M_PI * freq * t) > 0 ? amplitude : -amplitude); // square wave as a function of time
	}
}

void getBuffersWithHeader(int sample_rate, float duration_seconds, uint8_t** buffer, int16_t** sample_buffer) {
	int totalSamples = (int)(sample_rate * duration_seconds);
	int dataSize = totalSamples * sizeof(int16_t);
	int totalSize = sizeof(WAVHeader) + dataSize;

	*buffer = (uint8_t*)malloc(totalSize); // pointer to block of memory to store the generated sound buffer
	*sample_buffer = (int16_t*)(*buffer + sizeof(WAVHeader)); // pointer to an offset in the buffer to start storing sound data (after header)

	writeWAVHeader(*buffer, dataSize, 1, sample_rate);
}

void playWinSound(float duration) {
	uint8_t* buffer;
	int16_t* sample_buffer;
	getBuffersWithHeader(SAMPLE_RATE, duration, &buffer, &sample_buffer);

	// Winning melody: C6 G5 F5 C5 x6
	double tones[24] = {
			1046.50, 783.99, 698.46, 523.25, /**/ 1046.50, 783.99, 698.46, 523.25, /**/ 1046.50, 783.99, 698.46, 523.25,
			1046.50, 783.99, 698.46, 523.25, /**/ 1046.50, 783.99, 698.46, 523.25, /**/ 1046.50, 783.99, 698.46, 523.25
	};

	int samplesPerTone = SAMPLE_RATE * duration / 24;
	for (int i = 0; i < 24; i++) {
		generateSquareTone(sample_buffer, tones[i], duration / 24, i * samplesPerTone, SAMPLE_RATE, AMPLITUDE);
	}

	PlaySound((LPCSTR)buffer, NULL, SND_MEMORY | SND_ASYNC); // SND_MEMORY flag = play directly from memory. SND_ASYNC flag plays asynchronously
	free(buffer); // release memory
}

void playLoseSound(float duration) {
	uint8_t* buffer;
	int16_t* sample_buffer;
	getBuffersWithHeader(SAMPLE_RATE, duration, &buffer, &sample_buffer);

	// Losing melody: B5 G4b E4b C4 x6
	double tones[4] = {
		493.88, 369.99, 311.13, 261.63
	};

	int samplesPerTone = SAMPLE_RATE * duration / 4;
	for (int i = 0; i < 4; i++) {
		generateSquareTone(sample_buffer, tones[i], duration / 4, i * samplesPerTone, SAMPLE_RATE, AMPLITUDE);
	}

	PlaySound((LPCSTR)buffer, NULL, SND_MEMORY | SND_ASYNC); // SND_MEMORY flag = play directly from memory. SND_ASYNC flag plays asynchronously
	free(buffer); // release memory
}
