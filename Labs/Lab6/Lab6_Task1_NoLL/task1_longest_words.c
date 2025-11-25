/*
* task1_longest_words.c
* 2025F COMP-166 Lab 6 - Thomas Boland - C0556991
*
* This program finds and displays the longest word either from a
* file specified as a command line argument or from stdin.
*
*/

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "words_list.h"

void outputHelp();
bool isAlphaChar(int c);

int main(int argc, char* argv[]) {
	// check for help flag /? or too many arguments, then display help
	bool show_help = false;
	for (int i = 0; i < argc; i++) {
		if (strcmp(argv[i], "/?") == 0) {
			show_help = true;
			break;
		}
	}
	if (show_help || argc > 2) {
		outputHelp();
		return EXIT_SUCCESS;
	}
	
	// check for command line argument file name
	bool input_from_file = argc == 2;

	// set up words list structure
	LongestWordsList words;
	words.words = NULL;
	words.length = 0;
	words.count = 0;

	// either open file or use stdin
	FILE* input_stream = stdin;
	if (input_from_file) {
		fopen_s(&input_stream, argv[1], "r");
		if (input_stream == NULL) {
			perror("Error opening input file");
			return EXIT_FAILURE;
		}
	}

	// read words
	if (LongestWordsList_readFromStream(input_stream, &words) != 0) {
		LongestWordsList_clear(&words);
		if (input_from_file) {
			fclose(input_stream);
		}
		return EXIT_FAILURE;

	}

	// close file if used
	if (input_from_file) {
		fclose(input_stream);
	}

	// display the list of longest words.
	printf("The longest word(s) were %d characters long. Found %d words:\n", words.length, LongestWordsList_count(&words));
	LongestWordsList_outputToStream(stdout, &words);

	return EXIT_SUCCESS;
}

void outputHelp() {
	puts("Find and display the longest word(s) from a given input file or from stdin.");
	puts("A word consists of a sequence of alphabetic characters [A-Za-z] separated by");
	puts("any other character. If multiple words are tied for longest, unique words will");
	puts("be displayed in alphabetical order.");
	puts("");
	puts("Usage: lab6_task1 [input_file]");
	puts("  [input_file]   Path to file containing words. If input file not provided,");
	puts("                   read words from stdin instead.");
	puts("  /?             Display this help page.\n");
}