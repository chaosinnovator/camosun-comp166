/*
* log_summary_processing.c
* 2025F COMP-166 Lab 5 - Thomas Boland - C0556991
* 
* Contains function implementations for processing, populating, and
* outputting LogSummary structures.
* 
*/

#include <errno.h>
#include <float.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include "log_summary.h"

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

int processLogDataFromFile(const char* file_name, LogSummary* log_summary, int max_sensors) {
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
				// check next char without consuming it
				int c = fgetc(fptr);
				fseek(fptr, -1, SEEK_CUR);

				// if wrong character incountered:
				if (c != '\n' && c != EOF) {
					fprintf(stderr, "Error while processing file \"%s\":\nLine %d contains an entry that couldn't be parsed.\n", file_name, line + 1);
					return -1;
				}

				// if not first line:
				if (line != 0) {
					fprintf(stderr, "Error while processing file \"%s\":\nLine %d contains only %d data entries (%d required).\n", file_name, line + 1, sensor_id + 1, log_summary->n_sensors);
					return -1;
				}

				// if first line, then expect one fewer sensors/columns than this on subsequent lines:
				log_summary->n_sensors--;
				break;
			}

			// not yet end of line. if first line and <MAX_SENSORS:
			if (line == 0 && log_summary->n_sensors <= max_sensors) {
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