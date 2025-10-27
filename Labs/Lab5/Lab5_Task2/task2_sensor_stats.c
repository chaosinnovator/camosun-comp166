/*
* 2025F COMP-166 Lab 5 - Thomas Boland - C0556991
*
* This program calculates statistics from a given sensor log file and outputs
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
* ./echofile <input_file> [output_file]
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
#include <math.h>

#define MAX_LOG_LINE_LENGTH 500
// line length 500 chars - 20 chars for "[timestamp]" and "\n" = 480 chars / [min 2 chars per data] = 240 max # sensors
// would be better to dynamically-allocate a sensor array but we haven't covered malloc/realloc/free yet.
#define MAX_SENSORS (MAX_LOG_LINE_LENGTH - 20) / 2

void outputHelp();
bool checkOutputPathValid(const char* file_name);
int processLogDataFromFile(const char* file_name, double sensor_stats[][2], int* n_sensors, char* max_timestamp, char* min_timestamp, double* max_value, double* min_value);
int outputStats(FILE* stream, double sensor_stats[][2], int* n_sensors, char* max_timestamp, char* min_timestamp, double* max_value, double* min_value);

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
	if (show_help || argc < 2 || argc > 3) {
		outputHelp();
		return EXIT_SUCCESS;
	}

	// check validity of output_file if provided
	bool output_to_file = argc == 3;
	if (output_to_file && !checkOutputPathValid(argv[2])) {
		fprintf(stderr, "Unable to write to provided output_path \"%s\"", argv[2]);
		return EXIT_FAILURE;
	}

	// set up stats data structures
	// TODO use a struct if we learn it in time
	double sensor_stats[MAX_SENSORS][2];
	int n_sensors = 1;
	double max_value = DBL_TRUE_MIN;
	char max_timestamp[20]; // "2020-11-17T16:55:00\0" <-- 20 chars long
	double min_value = DBL_MAX;
	char min_timestamp[20];
	// open file and process data
	//   if failure, exit (processLogDataFromFile already prints error message to stderr
	if (processLogDataFromFile(argv[1], &sensor_stats, &n_sensors, &max_timestamp, &min_timestamp, &max_value, &min_value) != 0) {
		return EXIT_FAILURE;
	}

	// if output_to_file, open output file
	//   if failure, print to stderr and exit
	//   output report to file
	//   close file
	// else output report to stdout
	if (output_to_file) {
		FILE* fptr = fopen(argv[2], "w");
		if (!fptr) {
			fprintf(stderr, "An error occurred opening output file \"%s\" for writing.", argv[2]);
			return EXIT_FAILURE;
		}
		if (outputStats(fptr, &sensor_stats, &n_sensors, &max_timestamp, &min_timestamp, &max_value, &min_value) != 0) {
			fprintf(stderr, "An error occurred while writing to output file \"%s\"", argv[2]);
			fclose(fptr);
			return EXIT_FAILURE;
		}
		fclose(fptr);
	}
	else {
		if (outputStats(stdout, &sensor_stats, &n_sensors, &max_timestamp, &min_timestamp, &max_value, &min_value) != 0) {
			puts("An error occurred while writing output to stdout.");
			return EXIT_FAILURE;
		}
	}

	return EXIT_SUCCESS;
}

void outputHelp() {
	puts("Calculates statistics from a given sensor log file and outputs a statistics report to");
	puts("stdout or to a given file.");
	puts("Usage: lab5_task2 <input_file> [output_file]");
	puts("  <input_file>   Path to log file to open and compute statistics. File must contain lines like:");
	puts("                   [timestamp] [sensor_1] [sensor_2] ... [sensor_n]\\n");
	puts("                 where sensor values are floating-point values separated by whitespace and each");
	puts("                 line is a maximum of 500 characters long.");
	puts("  [output_file]  Optional file to write statistics report to.");
	puts("  /?             Display this help page.\n");
}

/**
* @brief Checks that output file is a valid path and that there are write permissions.
* @param file_name String containing file path
* @return Returns true if vaild path and write permission, false otherwise.
*/
bool checkOutputPathValid(const char* file_name) {
	FILE* fptr = fopen(file_name, "w");
	if (!fptr) {
		return false;
	}
	fclose(fptr);
	return true;
}

int processLogDataFromFile(const char* file_name, double sensor_stats[][2], int* n_sensors, char* max_timestamp, char* min_timestamp, double* max_value, double* min_value) {
	FILE* fptr = fopen(file_name, "r");
	if (!fptr) {
		fprintf(stderr, "An error occurred while opening input file \"%s\"", file_name);
		return -1;
	}

	// for each line in file
	// initially stores a sum and sum of squares in each sensor_stats, then calculates average and stddev later.
	char line_buffer[MAX_LOG_LINE_LENGTH]; // limits lines to a maximum length. malloc/realloc/free not introduced yet to have a flexible number of sensors.
	int line = 1;
	char line_timestamp[20];
	char* unconverted_line_buffer; // pointer to the current position along line_buffer, representing the un-parsed portion of the line
	char* new_unconverted_line_buffer;
	double new_value;
	while (fgets(line_buffer, MAX_LOG_LINE_LENGTH, fptr)) {
		// if line too long (line_buffer full and length without encountering '\n' is full buffer length), error
		if (strlen(line_buffer) == MAX_LOG_LINE_LENGTH - 1 && strcspn(line_buffer, "\n") == MAX_LOG_LINE_LENGTH - 1) {
			fprintf(stderr, "Error while processing file \"%s\": Line %d exceeds %d characters.\n", file_name, line, MAX_LOG_LINE_LENGTH);
			return -1;
		}
		// check that there are actually 19 characters before whitespace and save timestamp
		if (strcspn(line_buffer, " \t") != 19) {
			fprintf(stderr, "Error while processing file \"%s\": Timestamp not correct length on line %d.\n", file_name, line);
			return -1;
		}
		strncpy_s(line_timestamp, 20, line_buffer, 19);
		line_timestamp[19] = '\0'; // ensure line_timestamp is null-terminated.
		unconverted_line_buffer = line_buffer + 20;
		//   for sensor_num; s_n < n_sensors; s_n++
		for (int sensor_id = 0; sensor_id < *n_sensors; sensor_id++) {
			//parse double
			errno = 0; // strtof uses the errno pattern.
			new_value = strtod(unconverted_line_buffer, &new_unconverted_line_buffer);
			// if invalid double:
			//   print error and end
			if (new_unconverted_line_buffer == unconverted_line_buffer || new_unconverted_line_buffer == NULL || errno == ERANGE) {
				fprintf(stderr, "Error while processing file \"%s\": Unable to parse data at line %d col %d.\n", file_name, line, (int)(unconverted_line_buffer - line_buffer + 1));
				return -1;
			}
			unconverted_line_buffer = new_unconverted_line_buffer;
			// skip spaces or tabs
			unconverted_line_buffer += strspn(unconverted_line_buffer, " \t"); 
			// add value to sensor_stats[sensor_num] = {sum, sum_squares}
			sensor_stats[sensor_id][0] = line == 1 ? new_value : sensor_stats[sensor_id][0] + new_value;
			sensor_stats[sensor_id][1] = line == 1 ? new_value * new_value : sensor_stats[sensor_id][1] + new_value * new_value;
			// check min/max:
			// if value new max, save max_value and max_timestamp
			if (new_value > *max_value) {
				*max_value = new_value;
				strncpy_s(max_timestamp, 20, line_timestamp, 19);
			}
			// if value new min, save min_value and min_timestamp
			if (new_value < *min_value) {
				*min_value = new_value;
				strncpy_s(min_timestamp, 20, line_timestamp, 19);
			}
			// if unconverted_char != \n TODO check for \n, or EOF
			if (*unconverted_line_buffer != '\n' && *unconverted_line_buffer != '\0') {
				// if line==1 n_sensors = sensor_num+1
				if (line == 1) {
					*n_sensors = sensor_id + 2;
				}
				// else if s_n == n_sensors - 1: -> too many sensor values in this line. print error and end
				else if (sensor_id == *n_sensors - 1) {
					fprintf(stderr, "Error while processing file \"%s\": Line %d contains more than %d data entries.\n", file_name, line, *n_sensors);
					return -1;
				}
			}
			// else if s_n != n_sensors - 1 -> encountered newline but not enough sensor values in this line, print error and end
			else if (sensor_id != *n_sensors - 1) {
				fprintf(stderr, "Error while processing file \"%s\": Line %d contains only %d data entries (%d required).\n", file_name, line, sensor_id + 1, *n_sensors);
				return -1;
			}
		}
		line++;
	}

	line--;

	// handle case of no lines
	if (line == 0) {
		fprintf(stderr, "Error while processing file \"%s\": No lines processed.\n", file_name);
		return -1;
	}

	// each sensor_stat contains a sum and sum of squares.
	// need to calculate mean and std deviation for each sensor.
	for (int sensor_id = 0; sensor_id < *n_sensors; sensor_id++) {
		// mean = sum/n
		sensor_stats[sensor_id][0] = sensor_stats[sensor_id][0] / line;
		// stddev = sqrt( (sumsq - n*mean^2)/(n-1) ) <-- calculate this second since we need mean and sumsq
		sensor_stats[sensor_id][1] = sqrt((sensor_stats[sensor_id][1] - line * sensor_stats[sensor_id][0] * sensor_stats[sensor_id][0]) / (line - 1));
	}

	fclose(fptr);
	return 0;
}

int outputStats(FILE* stream, double sensor_stats[][2], int* n_sensors, char* max_timestamp, char* min_timestamp, double* max_value, double* min_value) {
	// output min and max
	fprintf(stream, "Maximum recorded at %s (%f)\n", max_timestamp, *max_value);
	fprintf(stream, "Minimum recorded at %s (%f)\n", min_timestamp, *min_value);

	// output mean and stddev for each sensor
	for (int sensor_id = 0; sensor_id < *n_sensors; sensor_id++) {
		fprintf(stream, "\nSensor %d:\n- mean: %f\n- deviation: %f\n", sensor_id + 1, sensor_stats[sensor_id][0], sensor_stats[sensor_id][1]);
	}

	return 0;
}