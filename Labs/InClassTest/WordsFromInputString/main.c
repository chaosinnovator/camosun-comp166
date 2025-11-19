#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void printSeparateWords(const char* inputString);

int main() {
	char sampleString[] = "This is a long string with multiple words in it.";

	printSeparateWords(sampleString);

	return EXIT_SUCCESS;
}

void printSeparateWords(const char* inputString) {
	char* word;
	char* context = NULL;
	word = strtok_s(inputString, " ", &context);

	while (word != NULL) {
		puts(word);
		word = strtok_s(NULL, " ", &context);
	}
}