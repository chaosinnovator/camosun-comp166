/*
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
#include <math.h>

#define MAX_LOG_LINE_LENGTH 500
// line length 500 chars - 20 chars for "[timestamp]" and "\n" = 480 chars / [min 2 chars per data] = 240 max # sensors
// would be better to dynamically-allocate a sensor array but we haven't covered malloc/realloc/free yet.
// lab instructions specify max 8 sensors, so we'll use that.
#define MAX_SENSORS 8

typedef struct {
	double sum, sum_squares;
	double mean, stddev;
	int count;
} SensorStats;

typedef struct {
	char timestamp[20];
	double value;
} Extrema;

typedef struct {
	int n_sensors;
	SensorStats* sensor_stats;
	Extrema max, min;
} LogSummary;

void outputHelp();
bool checkOutputPathValid(const char* file_name);
int processLogDataFromFile(const char* file_name, LogSummary* log_summary);
void calculateLogMeans(LogSummary* log_summary);
void calculateLogStdDevs(LogSummary* log_summary);
int outputStats(FILE* stream, const LogSummary log_summary);

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
		fprintf(stderr, "Unable to write to provided output_path \"%s\": ", argv[2]);
		perror(errno);
		return EXIT_FAILURE;
	}

	// set up stats data structures
	SensorStats sensor_stats[MAX_SENSORS];
	LogSummary summary;
	summary.sensor_stats = sensor_stats;

	// open file and process data
	//   if failure, exit (processLogDataFromFile already prints error message to stderr
	if (processLogDataFromFile(argv[1], &summary) != 0) {
		return EXIT_FAILURE;
	}

	calculateLogMeans(&summary);
	calculateLogStdDevs(&summary);

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
		if (outputStats(fptr, summary) != 0) {
			fprintf(stderr, "An error occurred while writing to output file \"%s\": ", argv[2]);
			perror(ferror(fptr));
			fclose(fptr);
			return EXIT_FAILURE;
		}
		fclose(fptr);
	}
	else {
		if (outputStats(stdout, summary) != 0) {
			puts("An error occurred while writing output to stdout.");
			return EXIT_FAILURE;
		}
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
	puts("                 If input file not provided, read sensor log data from stdin.");
	puts("  [output_file]  Optional file to write statistics report to.");
	puts("  /?             Display this help page.\n");
}

/**
* @brief Checks that output file is a valid path and that there are write permissions.
* @param file_name String containing file path
* @return Returns true if vaild path and write permission, false otherwise.
*/
bool checkOutputPathValid(const char* file_name) {
	FILE* fptr = fopen(file_name, "r+");
	if (!fptr) {
		return false;
	}
	fclose(fptr);
	return true;
}

int processLogDataFromFile(const char* file_name, LogSummary* log_summary) {
	FILE* fptr = fopen(file_name, "r");
	if (!fptr) {
		fprintf(stderr, "An error occurred while opening input file \"%s\": ", file_name);
		perror(errno);
		return -1;
	}

	// line length doesn't matter, we can process the file in smaller parts than line-by-line.
	//   until EOF:
	//     read from file for timestamp.
	//     read until whitespace (not newline)
	//      -> parse data, check extrema, increment sensor count
	//      -> check n_sensor, check if not enough/too many sensors @ end of line.
	//     read until \n and repeat
	int sensor_id = 0;
	log_summary->n_sensors = 1;
	log_summary->max.value = -DBL_MAX;
	log_summary->min.value = DBL_MAX;
	int line = 0;
	char line_timestamp[20];
	double new_sensor_value = 0;
	while (true) {
		if (feof(fptr)) {
			break;
		}

		// read timestamp (skip leading ' ' or '\t')
		if (fscanf_s(fptr, "%19[^ \t\n]", line_timestamp, 20) != 1) {
			fprintf(stderr, "Error while processing file \"%s\": Unable to read timestamp on line %d.\n", file_name, line + 1);
			return -1;
		}

		line_timestamp[19] = '\0';

		// check timestamp length
		if (strlen(line_timestamp) != 19) {
			fprintf(stderr, "Error while processing file \"%s\": Timestamp not correct length on line %d.\n", file_name, line + 1);
			return -1;
		}

		// read sensor values
		for (int sensor_id = 0; sensor_id < log_summary->n_sensors; sensor_id++) {
			if (fscanf_s(fptr, "%*[ \t]%lf", &new_sensor_value) != 1) {
				// reached end of line or a wrong character
				// if not first line:
				if (line != 0) {
					fprintf(stderr, "Error while processing file \"%s\":\nLine %d contains only %d data entries (%d required), or contains an entry that couldn't be parsed.\n", file_name, line + 1, sensor_id + 1, log_summary->n_sensors);
					return -1;
				}
				// if first line:
				log_summary->n_sensors--;
				break;
			}

			// not yet end of line. if first line and <MAX_SENSORS:
			if (line == 0 && log_summary->n_sensors <= MAX_SENSORS) {
				log_summary->n_sensors++;
			}

			// process new sensor value (if line == 0, sum/sumsquares still needs initialization)
			log_summary->sensor_stats[sensor_id].sum =
				(line == 0 ? new_sensor_value : log_summary->sensor_stats[sensor_id].sum + new_sensor_value);
			log_summary->sensor_stats[sensor_id].sum_squares =
				(line == 0 ? new_sensor_value * new_sensor_value : log_summary->sensor_stats[sensor_id].sum_squares + new_sensor_value * new_sensor_value);
			log_summary->sensor_stats[sensor_id].count = line + 1;

			// check min/max:
			if (new_sensor_value > log_summary->max.value) {
				log_summary->max.value = new_sensor_value;
				strncpy_s(log_summary->max.timestamp, 20, line_timestamp, 19);
			}
			if (new_sensor_value < log_summary->min.value) {
				log_summary->min.value = new_sensor_value;
				strncpy_s(log_summary->min.timestamp, 20, line_timestamp, 19);
			}
		}

		// read characters until \n or EOF.
		// if we encounter a character that is not whitespace first, then there are too many sensors.
		int c; // needs to be int not char for EOF detection since EOF=-1
		while (true) {
			c = fgetc(fptr);
			if (c == EOF || c == '\n') {
				break;
			}
			
			if (c != '\t' && c != ' ') {
				fprintf(stderr, "Error while processing file \"%s\": Line %d contains more than %d data entries.\n", file_name, line + 1, log_summary->n_sensors);
				return -1;
			}
		}

		if (c != EOF) {
			line++;
		}
	}

	// handle case of no lines
	if (line == 0) {
		fprintf(stderr, "Error while processing file \"%s\": No lines processed.\n", file_name);
		return -1;
	}

	fclose(fptr);
	return 0;
}

void calculateLogMeans(LogSummary* log_summary) {
	for (int i = 0; i < log_summary->n_sensors; i++) {
		log_summary->sensor_stats[i].mean = log_summary->sensor_stats[i].sum / log_summary->sensor_stats[i].count;
	}
}

void calculateLogStdDevs(LogSummary* log_summary) {
	for (int i = 0; i < log_summary->n_sensors; i++) {
		// stddev = sqrt( (sumsq - sum^2)/(n*(n-1)) ) <-- calculate this second since we need mean and sumsq
		log_summary->sensor_stats[i].stddev = sqrt((log_summary->sensor_stats[i].count * log_summary->sensor_stats[i].sum_squares - log_summary->sensor_stats[i].sum * log_summary->sensor_stats[i].sum) / (log_summary->sensor_stats[i].count * (log_summary->sensor_stats[i].count - 1)));
	}
}

int outputStats(FILE* stream, const LogSummary log_summary) {
	// output min and max
	fprintf(stream, "Maximum recorded at %s (%f)\n", log_summary.max.timestamp, log_summary.max.value);
	fprintf(stream, "Minimum recorded at %s (%f)\n", log_summary.min.timestamp, log_summary.min.value);

	// output mean and stddev for each sensor
	for (int sensor_id = 0; sensor_id < log_summary.n_sensors; sensor_id++) {
		fprintf(stream, "\nSensor %d:\n- mean: %f\n- deviation: %f\n", sensor_id + 1, log_summary.sensor_stats[sensor_id].mean, log_summary.sensor_stats[sensor_id].stddev);
	}

	return 0;
}