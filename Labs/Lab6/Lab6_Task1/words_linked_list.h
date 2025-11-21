/*
* words_linked_list.h
* 2025F COMP-166 Lab 6 - Thomas Boland - C0556991
* 
* Contains structure and function prototypes for manipulating a linked list of words.
*/

#include <stdlib.h>

typedef struct WordNode {
	char* word;
	struct WordNode* next;
} WordNode;

typedef struct {
	WordNode* head;
	WordNode* tail;
} WordLinkedList;

typedef struct {
	WordLinkedList words;
	int length; // length of each word in the list
} LongestWordsList;

void LongestWordsList_clear(LongestWordsList* list);
int LongestWordsList_count(const LongestWordsList* list);
int LongestWordsList_appendIfUnique(LongestWordsList* list, const char* word);