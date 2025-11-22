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
#include "words_linked_list.h"

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
	WordLinkedList list;
	list.head = NULL;
	list.tail = NULL;
	LongestWordsList words;
	words.words = list;
	words.length = 0;

	// either open file or use stdin
	FILE* input_stream = stdin;
	if (input_from_file) {
		fopen_s(&input_stream, argv[1], "r");
		if (input_stream == NULL) {
			perror("Error opening input file");
			return EXIT_FAILURE;
		}
	}
	
	int new_word_length = 0;
	int new_word_max_length = 1;
	char* new_word = (char*)malloc(new_word_max_length);
	if (new_word == NULL) {
		fprintf(stderr, "Unable to allocate sufficient memory to process word from input.\n");
		return EXIT_FAILURE;
	}
	new_word[0] = '\0';
	int c;
	// until input ends:
	while (!feof(input_stream)) {
		// read word into dynamic char* (skip characters that are not A-Z or a-z, then read until next non A-Z or a-z)
		c = fgetc(input_stream);

		// this check has to be first, because c could be EOF and isAlphaChar requires a char. EOF = -1 and would cast wrongly.
		if (c == EOF || c == '\n') {
			// end of input or line
			if (new_word_length == 0) {
				// no word read, end of input
				continue;
			}
		}

		if (isAlphaChar(c)) {
			++new_word_length;
			if (new_word_length >= new_word_max_length) {
				new_word_max_length = new_word_length + 1;
				new_word = realloc(new_word, new_word_max_length);
				if (new_word == NULL) {
					// failed to increase new_word size. per lab instructions, we should stop here and print what was processed so far.
					fprintf(stderr, "Unable to allocate sufficient memory to process word from input.\n");
					break;
				}
			}
			new_word[new_word_length - 1] = (char)c;
			new_word[new_word_length] = '\0';
			continue;
		}

		// all other cases mean the word ended.
		// add word to longest words list and clear new_word.
		if (new_word_length < words.length) {
			new_word[0] = '\0';
			new_word_length = 0;
			continue;
		}

		// check length against longest words list
		//   if longer, clear list and add word
		//   if same length, add word
		if (new_word_length > words.length) {
			// new longest word
			LongestWordsList_clear(&words);
			words.length = new_word_length;
		}

		LongestWordsList_appendIfUnique(&words, new_word); // this copies new_word, so it's safe to reuse new_word buffer
		new_word[0] = '\0';
		new_word_length = 0;
	}

	// close file if used
	if (input_from_file) {
		fclose(input_stream);
	}

	// display the list of longest words.
	printf("The longest word(s) were %d characters long. Found %d words:\n", words.length, LongestWordsList_count(&words));
	for (WordNode* current = words.words.head; current != NULL; current = current->next) {
		printf("%s\n", current->word);
	}

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

bool isAlphaChar(int c) {
	return (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z');
}