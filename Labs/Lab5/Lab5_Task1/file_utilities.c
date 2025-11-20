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

void printErrno(int error_number, FILE* output_stream) {
	char err_buf[256];
	strerror_s(err_buf, sizeof(err_buf), error_number);
	fprintf(output_stream, "%s\n", err_buf);
}

int filePeakNextChar(FILE* fptr) {
	int c = fgetc(fptr);
	ungetc(c, fptr);
	return c;
}

bool checkOutputPathValid(const char* file_name) {
	FILE* fptr = fopen(file_name, "r+");
	if (!fptr) {
		return false;
	}
	fclose(fptr);
	return true;
}