/*
* file_utilities.h
* 2025F COMP-166 Lab 5 - Thomas Boland - C0556991
*
* Contains function prototypes for some common file utilites.
*
*/

#include <stdbool.h>
#include <stdio.h>

/**
* @brief Returns the next character in the file without advancing the file pointer.
* @param fptr Pointer to open FILE stream
* @return Returns the next character as an int or EOF
*/
int filePeekNextChar(FILE* fptr);

/**
* @brief Checks that output file is a valid path and that there are write permissions.
* @param file_name String containing file path
* @return Returns true if vaild path and write permission, false otherwise.
* 
* Note: Current implementation of this function will create the file if it does not exist.
*/
bool checkOutputPathValid(const char* file_name);