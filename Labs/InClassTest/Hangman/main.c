/**
* Hangman game - Thomas Boland
*/

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define SECRET_MAX_LENGTH 64
#define MAX_WRONG_GUESSES 6 // head, torso, left arm, right arm, left leg, right leg = 6
#define N_VALID_GUESSES 26

void outputHelp();
int convertAndValidateSecret(char* secret);
bool isLetter(char c);
void displayBoard(int wrong_guesses, char* past_guesses, char* guess);
int getGuessInput(char* new_guess, char* past_guesses);
void appendPastGuess(char new_guess, char* past_guesses);
int revealGuess(char new_guess, char* secret, char* guess);

int main(int argc, char* argv[]) {
	// need 1 arg for secret word/phrase
	if (argc != 2) {
		outputHelp();
		return EXIT_FAILURE;
	}

	char* secret = argv[1];

	// convert secret to uppercase.
	// make sure secret only contains uppercase letters or space
	// secret max length
	if (convertAndValidateSecret(secret) != 0) {
		printf("Provided secret word or phrase is not valid. Must contain only letters or spaces, and must be at most %d characters long.", (SECRET_MAX_LENGTH - 1));
		return EXIT_FAILURE;
	}

	// initialiaze guess array with '_' in every position of secret containing a letter and ' ' for every position containing a space
	char guess[SECRET_MAX_LENGTH];
	for (int i = 0; i < strlen(secret); i++) {
		guess[i] = (isLetter(secret[i]) ? '_' : ' ');
	}
	guess[strlen(secret)] = '\0'; // null-terminate guess array -> C-string

	// initialize past guess array (26 letters in alphabet)
	char past_guesses[N_VALID_GUESSES];
	for (int i = 0; i < N_VALID_GUESSES; i++) {
		past_guesses[i] = 0;
	}
	// initialize wrong_guesses = 0
	int wrong_guesses = 0;

	// game loop:
	bool is_playing = true;
	char new_guess;
	while (is_playing) {
		//   display board, wrong guesses, blanks/correct guesses.
		displayBoard(wrong_guesses, past_guesses, guess);
		//   if wrong_guesses > MAX_WRONG_GUESSES then display lose message and break;
		if (wrong_guesses >= MAX_WRONG_GUESSES) {
			printf("You lost. The secret phrase was:\n%s", secret);
			break;
		}
		// check if win (guess has no '_' left)
		is_playing = false;
		for (int i = 0; i < strlen(secret); i++) {
			if (guess[i] == '_') {
				is_playing = true;
				break;
			}
		}
		if (!is_playing) {
			puts("You won!");
			break;
		}

		//   accept guess input (1 char, convert to uppercase, guess must be a letter, guess not in past guesses)
		if (getGuessInput(&new_guess, past_guesses) != 0) {
			fputs("Unrecoverable error while getting input.", stderr);
			return EXIT_FAILURE;
		}
		// add newGuess to past_guesses
		appendPastGuess(new_guess, past_guesses);
		//   search through secret, for each position containing the guess set that letter in guess array, count num matches
		if (!revealGuess(new_guess, secret, guess)) {
			//   if num_matches = 0 then increment wrong guess
			wrong_guesses++;
			continue;
		}
	}

	return EXIT_SUCCESS;
}

/*
* @brief Display command line documentation
*/
void outputHelp() {
	puts("Usage: Hangman [secret_phrase]");
	puts("");
	puts(" [secret_phrase]  The secret word or phrase to be guessed. Must contain");
	puts("                   only letters and spaces.");
}

/*
* @brief Converts the input string to uppercase letters, ensure string is not too long
*        or contains characters other than letters or spaces
* @param secret Input string to process
* @returns Returns zero if valid, non-zero if invalid.
*/
int convertAndValidateSecret(char* secret) {
	for (int i = 0; i < strlen(secret); i++) {
		// too long, invalid
		if (i > (SECRET_MAX_LENGTH - 2)) {
			return -1;
		}

		// convert lowercase to uppercase
		if (secret[i] >= 'a' && secret[i] <= 'z') {
			secret[i] -= 32;
		}

		// if not letter or space, invalid
		if (secret[i] != ' ' && !isLetter(secret[i])) {
			return -1;
		}
	}

	return 0;
}

/*
* @brief Check if provided character is a letter
* @param c Character to check
* @returns Return true if letter, false otherwise.
*/
bool isLetter(char c) {
	return (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z');
}

/*
* @brief Clear console and output an ASCII-art game board
* @param wrong_guesses Number of wrong guesses made so far
* @param past_guesses Array containing each previous guess
* @param guess String with revealed letters, '_' for non-revealed letters
*/
void displayBoard(int wrong_guesses, char* past_guesses, char* guess) {
	system("cls"); // windows only, clear console
	puts("Past guesses:");
	for (int i = 0; i < N_VALID_GUESSES; i++) {
		if (past_guesses[i] == 0) {
			break;
		}
		printf("%c ", past_guesses[i]);
	}
	printf("\nWrong guesses: %d\n\n", wrong_guesses); // TODO replace with ASCII art hangman board
	// head, torso, left arm, right arm, left leg, right leg
	/*
	* +----+
	* |    |
	* |    O
	* |   /|\
	* |   / \
	* |
	* +----------
	*
	*/
	printf("+----+\n");
	printf("|    |\n");
	printf("|    %c\n", (wrong_guesses >= 1 ? 'O' : ' '));
	printf("|   %c%c%c\n", (wrong_guesses >= 5 ? '/' : ' '), (wrong_guesses >= 2 ? '|' : ' '), (wrong_guesses >= 6 ? '\\' : ' '));
	printf("|   %c %c\n", (wrong_guesses >= 3 ? '/' : ' '), (wrong_guesses >= 4 ? '\\' : ' '));
	printf("|\n");
	printf("+----------\n\n");
	for (int i = 0; i < strlen(guess); i++) {
		printf("%c ", guess[i]);
	}
	putchar('\n');
}

/*
* @brief Gets input of a single character from stdin, forces user to retry until valid input.
*        Guess must be a letter and not already guessed.
* @param new_guess Pointer (char*) to where to store new input guess
* @param past_guesses Array containing each previous guess
* @returns non-zero if not successful.
*/
int getGuessInput(char* new_guess, char* past_guesses) {
	char input_buffer[32];
	while (true) {
		printf("Enter guess: ");
		if (fgets(input_buffer, 32, stdin) == NULL) {
			return -1;
		}
		if (strcspn(input_buffer, "\n") != 1) {
			puts("Guess must only contain 1 character.");
			continue;
		}
		if (!isLetter(input_buffer[0])) {
			puts("Guess must be a letter.");
			continue;
		}

		// Convert to uppercase.
		if (input_buffer[0] >= 'a' && input_buffer[0] <= 'z') {
			input_buffer[0] -= 32;
		}

		// check if guess in past guesses
		bool in_past_guesses = false;
		for (int i = 0; i < N_VALID_GUESSES; i++) {
			if (past_guesses[i] == 0) {
				break;
			}

			if (past_guesses[i] != input_buffer[0]) {
				continue;
			}

			in_past_guesses = true;
		}

		if (in_past_guesses) {
			puts("You've already guessed that! Try a different guess.");
			continue;
		}

		// valid input
		(*new_guess) = input_buffer[0];
		break;
	}
	return 0;
}

/*
* @brief Add a character to the end of char array (end padded with \0)
* @param new_guess The character to append
* @param past_guesses Array containing each previous guess
*/
void appendPastGuess(char new_guess, char* past_guesses) {
	for (int i = 0; i < N_VALID_GUESSES; i++) {
		if (past_guesses[i]) {
			continue;
		}

		past_guesses[i] = new_guess;
		break;
	}
}

/*
* @brief Write characters from secret string that match the new guess to the string of revealed guesses
* @param new_guess The guessed character
* @param secret String containing correct answer
* @param guess String with revealed letters, '_' for non-revealed letters
* @returns Number of characters in secret that match the guess
*/
int revealGuess(char new_guess, char* secret, char* guess) {
	int n_matches = 0;
	for (int i = 0; i < strlen(secret); i++) {
		if (secret[i] != new_guess) {
			continue;
		}

		if (guess[i] != '_') { // already revealed this position
			continue;
		}

		guess[i] = new_guess;
		n_matches++;
	}

	return n_matches;
}
