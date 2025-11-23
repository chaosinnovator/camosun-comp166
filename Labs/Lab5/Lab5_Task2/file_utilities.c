/*
* file_utilities.c
* 2025F COMP-166 Lab 5 - Thomas Boland - C0556991
*
* Contains function implementations for some common file utilites.
*
*/

#include <stdio.h>
#include <string.h>
#include "file_utilities.h"

int filePeekNextChar(FILE* fptr) {
	int c = fgetc(fptr);
	if (c == EOF) {
		fseek(fptr, -1, SEEK_CUR);
		return EOF;
	}
	ungetc(c, fptr);
	return c;
}

bool checkOutputPathValid(const char* file_name) {
	FILE* fptr = fopen(file_name, "a");
	if (!fptr) {
		return false;
	}
	fclose(fptr);
	return true;
}