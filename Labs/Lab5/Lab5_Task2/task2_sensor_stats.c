/*
* task2_sensor_stats.c
* 2025F COMP-166 Lab 5 - Thomas Boland - C0556991
*
* This program calculates statistics from either stdin or a given sensor log file and outputs
* a statistics report to stdout or written to a given file. The sensor log
* file must contain lines formatted like:
* 
* [timestamp] [sensor-1] [sensor_2] [sensor_3] ... [sensor_n]
* 
* Where sensor values are floating-point values separated by whitespace
* and each line is a maximum of 500 characters including the ending newline.
* 
* If an output file is provided and the provided filename is unable to
* be opened in write mode, the log file will not be processed and an
* error message displayed.
*
* The program accepts up to two arguments from the command line:
* ./echofile [input_file] [output_file]
*
* If provided arguments are invalid or any argument is /?, outputs a help description instead.
*
*/

#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <float.h>
#include <string.h>
#include <errno.h>
#include "log_summary.h"
#include "file_utilities.h"

// lab instructions specify max 8 sensors, so we'll use that.
#define MAX_SENSORS 8

void outputHelp();
bool checkOutputPathValid(const char* file_name);

int main(int argc, char* argv[]) {
	// check for presence of /? flag in args
	bool show_help = false;
	for (int i = 1; i < argc; i++) {
		if (strcmp(argv[i], "/?") == 0) {
			show_help = true;
			break;
		}
	}
	// if /? present or if not correct number of args, output help
	if (show_help || argc > 3) {
		outputHelp();
		return EXIT_SUCCESS;
	}

	bool input_from_file = argc >= 2;

	// check validity of output_file if provided
	bool output_to_file = argc == 3;
	if (output_to_file && !checkOutputPathValid(argv[2])) {
		perror("Unable to write to provided output_path");
		return EXIT_FAILURE;
	}

	// set up stats data structures
	SensorStats sensor_stats[MAX_SENSORS];
	LogSummary summary;
	summary.sensor_stats = sensor_stats;

	// open file (or use stdin) and process data
	//   if failure, exit (processLogDataFromFile already prints error message to stderr
	FILE* input_stream = stdin;
	if (input_from_file) {
		fopen_s(&input_stream, argv[1], "r");
		if (!input_stream) {
			perror("An error occurred while opening input file");
			return EXIT_FAILURE;
		}
	}
	if (processLogData(input_stream, &summary, MAX_SENSORS) != 0) {
		// error message already printed in processLogData
		return EXIT_FAILURE;
	}
	if (input_from_file) {
		fclose(input_stream);
	}

	calculateLogMeans(&summary);
	calculateLogStdDevs(&summary);

	// if output_to_file, open output file
	//   if failure, print to stderr and exit
	//   output report to file
	//   close file
	// else output report to stdout
	FILE* output_stream = stdout;
	if (output_to_file) {
		fopen_s(&output_stream, argv[2], "w");
		if (!output_stream) {
			perror("An error occurred opening output file for writing");
			return EXIT_FAILURE;
		}
	}
	if (outputStats(output_stream, summary) != 0) {
		perror("An error occurred while writing output");
		return EXIT_FAILURE;
	}
	if (output_to_file) {
		fclose(output_stream);
	}

	return EXIT_SUCCESS;
}

void outputHelp() {
	puts("Calculates statistics from a given sensor log file and outputs a statistics report to");
	puts("stdout or to a given file.");
	puts("Usage: lab5_task2 [input_file] [output_file]");
	puts("  [input_file]   Path to log file to open and compute statistics. File must contain lines like:");
	puts("                   [timestamp] [sensor_1] [sensor_2] ... [sensor_n]\\n");
	puts("                 where sensor values are floating-point values separated by whitespace and each");
	puts("                 line is a maximum of 500 characters long.");
	puts("                 If input file not provided, read sensor log data from stdin until blank line.");
	puts("  [output_file]  Optional file to write statistics report to.");
	puts("  /?             Display this help page.\n");
}