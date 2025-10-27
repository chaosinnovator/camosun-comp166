/*
* 2025F COMP-166 Lab 5 - Thomas Boland - C0556991
*
* This program outputs the contents of a list of input files to stdout.
* The contents of a file who's argument is preceeded by -a will be
*  interpreted as a list of additional file names to process
*  separated by \n.
* 
* If a file can't be opened, an error message is output to stderr
*  and processing continues with the next file name.
*
* The program accepts a non-determinate number of arguments from the command line:
* ./echofile [[[-a] file]...]
*
* If provided arguments are invalid or any argument is /?, output a help description instead.
*
*/

#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

void outputHelp();
int outputFile(const char* file_name);
int outputListFile(const char* file_name);

int main(int argc, char* argv[]) {
	// check for presence of /? flag in args
	bool show_help = false;
	for (int i = 1; i < argc; i++) {
		if (strcmp(argv[i], "/?") == 0) {
			show_help = true;
			break;
		}
	}
	// if /? present or if not enough args, output help
	if (show_help || argc < 2) {
		outputHelp();
		return EXIT_SUCCESS;
	}

	// for each arg:
	for (int i = 1; i < argc; i++) {
		//   if arg is -a:
		if (strcmp(argv[i], "-a") == 0) {
			//     if next arg not present, stop and output error
			i++;
			if (i >= argc) {
				// This check should probably be done before starting to output each file contents.
				fputs("A filename must be provided after -a", stderr);
				return EXIT_FAILURE;
			}
			//     call outputListFile()
			if (outputListFile(argv[i]) != 0) {
				fprintf(stderr, "Error while processing list file \"%s\"\n", argv[i]);
			}
			continue;
		}
		//   else:
		//     outputFile()
		if (outputFile(argv[i]) != 0) {
			fprintf(stderr, "Error while processing file \"%s\"\n", argv[i]);
		}
	}

	return EXIT_SUCCESS;
}

void outputHelp() {
	puts("Output the contents of one or more files to stdout.\n");
	puts("Usage: lab5_task1 ([-a] <filename>)...");
	puts("  <filename>     Path to file to open and output contents to stdout.");
	puts("  -a <filename>  The following filename after this flag will have its contents interpreted");
	puts("                  as a list of additional filenames to read separated by newlines.");
	puts("  /?             Display this help page.\n");
}

/**
* @brief Outputs the contents of the specified file to stdout.
* @param *filename String containing path to file to read
* @return Returns 0 if successful, non-zero if failed
*/
int outputFile(const char* filename) {
	// check validity of filename
	// check file exists and can be opened
	// open file
	FILE* fptr = fopen(filename, "r");
	if (!fptr) {
		return -1;
	}
	// for each char in file:
	//   putc(char) to stdout
	int c; // needs to be int not char for EOF detection since EOF=-1
	while (true) {
		c = fgetc(fptr);
		if (c == EOF) {
			break;
		}
		putchar(c);
	}
	putchar('\n');

	if (ferror(fptr) != 0) {
		fclose(fptr);
		return -1;
	}

	// close file
	fclose(fptr);
	return 0;
}

/**
* @brief Interprets the contents of a file as a list of filenames and outputs the content of each listed file to stdout.
* @param *filename String containing path to file to read
* @return Returns 0 if successful, non-zero if failed
*/
int outputListFile(const char* filename) {
	// check validity of filename
	// check file exists and can be opened
	// open file
	FILE* fptr = fopen(filename, "r");
	if (!fptr) {
		return -1;
	}
	// for each line in file:
	//   outputFile(line);
	char line_buffer[256]; // limits lines to a maximum of 256 characters. malloc/realloc/free not introduced yet.
	while (fgets(line_buffer, 256, fptr)) {
		line_buffer[strcspn(line_buffer, "\n")] = 0; // remove trailing newline
		if (outputFile(line_buffer) != 0) {
			fprintf(stderr, "Error while processing file \"%s\" (listed in \"%s\")\n", line_buffer, filename);
		}
	}
	// close file
	fclose(fptr);
	return 0;
}