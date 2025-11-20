/*
* file_utilities.h
* 2025F COMP-166 Lab 5 - Thomas Boland - C0556991
*
* Contains function prototypes for some common file utilites.
*
*/

#include <stdbool.h>
#include <stdio.h>

void printErrno(int error_number, FILE* output_stream);
int filePeakNextChar(FILE* fptr);

/**
* @brief Checks that output file is a valid path and that there are write permissions.
* @param file_name String containing file path
* @return Returns true if vaild path and write permission, false otherwise.
*/
bool checkOutputPathValid(const char* file_name);