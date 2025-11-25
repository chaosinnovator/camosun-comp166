/*
* words_linked_list.h
* 2025F COMP-166 Lab 6 - Thomas Boland - C0556991
* 
* Contains structure and function prototypes for manipulating a list of words.
*/

#include <stdlib.h>
#include <stdio.h>

typedef struct {
	char** words;
	int count;
	int length; // length of each word in the list
} LongestWordsList;

void LongestWordsList_clear(LongestWordsList* list);
int LongestWordsList_count(const LongestWordsList* list);
int LongestWordsList_appendIfUnique(LongestWordsList* list, const char* word);

int LongestWordsList_readFromStream(FILE* stream, LongestWordsList* list);
void LongestWordsList_outputToStream(FILE* stream, const LongestWordsList* list);