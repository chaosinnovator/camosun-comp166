/*
* words_linked_list.c
* 2025F COMP-166 Lab 6 - Thomas Boland - C0556991
*
* Contains function implementations for manipulating a list of words.
*/

#include <ctype.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "words_list.h"

void LongestWordsList_clear(LongestWordsList* list) {
	if (list == NULL || list->words == NULL) {
		return;
	}

	for (int i = 0; i < list->count; i++) {
		if (list->words[i] != NULL) {
			free(list->words[i]);
		}
	}

	free(list->words);

	list->count = 0;
	list->length = 0;
}

int LongestWordsList_count(const LongestWordsList* list) {
	if (list == NULL) {
		return 0;
	}

	return list->count;
}

// append word in alphabetical order, ignoring case, only if not already present
int LongestWordsList_appendIfUnique(LongestWordsList* list, const char* new_word) {
	if (list == NULL) {
		return -1;
	}
	if (list->words == NULL || list->count == 0) {
		// first word
		list->words = (char**)malloc(sizeof(char*));
		if (list->words == NULL) { // failed to allocate
			return -1;
		}
	}
	// iterate through list.
	//   if duplicate found (ignoring case), we're done (successful)
	//   if we find a word that is greater than the new word (ignoring case), insert before it
	for (int i = 0; i < list->count; i++) {
		// case-insensitive compare
		int cmp = strcmpi(list->words[i], new_word);
		if (cmp == 0) { // duplicate, we're done.
			return 0;
		}
		if (cmp < 0) { // keep looking
			continue;
		}
		// insert before i
		// make space
		char** newWordsArray = (char**)realloc(list->words, (list->count + 1) * sizeof(char*));
		if (newWordsArray == NULL) { // failed to append
			return -1;
		}
		list->words = newWordsArray;

		// shift elements up
		for (int j = list->count; j > i; j--) {
			list->words[j] = list->words[j - 1];
		}

		// copy new word
		list->words[i] = _strdup(new_word);
		if (list->words[i] == NULL) {
			return -1;
		}

		list->count++;
		return 0;
	}
	// insert at end
	// make space
	char** newWordsArray = (char**)realloc(list->words, (list->count + 1) * sizeof(char*));
	if (newWordsArray == NULL) { // failed to append
		return -1;
	}
	list->words = newWordsArray;
	// copy new word
	list->words[list->count] = _strdup(new_word);
	if (list->words[list->count] == NULL) {
		return -1;
	}

	list->count++;
	return 0;
}

int LongestWordsList_readFromStream(FILE* stream, LongestWordsList* list) {
	int new_word_length = 0;
	int new_word_max_length = 1;
	char* new_word = (char*)malloc(new_word_max_length);
	if (new_word == NULL) {
		fprintf(stderr, "Unable to allocate sufficient memory to process any words from input.\n");
		return 0;
	}
	new_word[0] = '\0';
	int c;
	// until input ends:
	while (!feof(stream)) {
		// read word into dynamic char* (skip characters that are not A-Z or a-z, then read until next non A-Z or a-z)
		c = fgetc(stream);

		// this check has to be first, because c could be EOF and isAlphaChar requires a char. EOF = -1 and would cast wrongly.
		if (c == EOF || c == '\n') {
			// end of input or line
			if (new_word_length == 0) {
				// no word read, end of input
				continue;
			}
		}

		if (isalpha(c)) { // NOTE: intended behaviour requires locale is 'C' or 'POSIX' so that isalpha only returns true for A-Z and a-z.
			++new_word_length;
			if (new_word_length >= new_word_max_length) {
				new_word_max_length = new_word_length + 1;
				char* old_word = new_word;
				new_word = realloc(new_word, new_word_max_length);
				if (new_word == NULL) {
					// failed to increase new_word size. per lab instructions, we should stop here and print what was processed so far.
					free(old_word);
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
		if (new_word_length < list->length) {
			new_word[0] = '\0';
			new_word_length = 0;
			continue;
		}

		// check length against longest words list
		//   if longer, clear list and add word
		//   if same length, add word
		if (new_word_length > list->length) {
			// new longest word
			LongestWordsList_clear(list);
			list->length = new_word_length;
		}

		LongestWordsList_appendIfUnique(list, new_word); // this copies new_word, so it's safe to reuse new_word buffer
		new_word[0] = '\0';
		new_word_length = 0;
	}

	return 0;
}

void LongestWordsList_outputToStream(FILE* stream, const LongestWordsList* list) {
	for (int i = 0; i < list->count; i++) {
		fprintf(stream, "%s\n", list->words[i]);
	}
}