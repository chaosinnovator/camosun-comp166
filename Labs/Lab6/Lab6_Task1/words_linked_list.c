/*
* words_linked_list.c
* 2025F COMP-166 Lab 6 - Thomas Boland - C0556991
*
* Contains function implementations for manipulating a linked list of words.
*/

#include <stdlib.h>
#include <string.h>
#include "words_linked_list.h"

void LongestWordsList_clear(LongestWordsList* list) {
	if (list == NULL) {
		return;
	}

	WordNode* current = list->words.head;
	while (current != NULL) {
		WordNode* toFree = current;
		current = current->next;
		if (toFree->word != NULL) {
			free(toFree->word);
		}
		free(toFree);
	}
	list->words.head = NULL;
	list->words.tail = NULL;
	list->length = 0;
}

int LongestWordsList_count(const LongestWordsList* list) {
	if (list == NULL) {
		return 0;
	}

	int count = 0;
	for (WordNode* current = list->words.head; current != NULL; current = current->next) {
		count++;
	}
	return count;
}

// append word in alphabetical order, ignoring case, only if not already present
int LongestWordsList_appendIfUnique(LongestWordsList* list, const char* new_word) {
	if (list == NULL) {
		return -1;
	}
	// iterate through list.
	//   if duplicate found (ignoring case), we're done (successful)
	//   if we find a word that is greater than the new word (ignoring case), insert before it
	WordNode* previous = NULL;
	WordNode* current = list->words.head;
	while (current != NULL) {
		// case-insensitive compare
		int cmp = strcmpi(current->word, new_word);
		if (cmp == 0) { // duplicate, we're done.
			return 0;
		}
		if (cmp < 0) { // keep looking
			previous = current;
			current = current->next;
			continue;
		}
		// insert before current
		WordNode* newNode = (WordNode*)malloc(sizeof(WordNode));
		if (newNode == NULL) { // failed to append
			return -1;
		}
		//copy string
		newNode->word = _strdup(new_word);
		if (newNode->word == NULL) {
			free(newNode);
			return -1;
		}

		newNode->next = current;
		if (previous != NULL) {
			previous->next = newNode;
		}
		else {
			list->words.head = newNode;
		}
		return 0;
	}
	// insert at end
	WordNode* newNode = (WordNode*)malloc(sizeof(WordNode));
	if (newNode == NULL) { // failed to append
		return -1;
	}
	//copy string
	newNode->word = _strdup(new_word);
	if (newNode->word == NULL) {
		free(newNode);
		return -1;
	}
	newNode->next = NULL;
	if (previous != NULL) {
		previous->next = newNode;
		list->words.tail = newNode;
	}
	else {
		list->words.head = newNode;
	}
	return 0;
}