/*
* log_summary_functions.c
* 2025F COMP-166 Lab 5 - Thomas Boland - C0556991
* 
* Contains function implementations for processing, populating, and
* outputting LogSummary structures.
* 
*/

#include <errno.h>
#include <float.h>
#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include "log_summary.h"
#include "file_utilities.h"

void calculateLogMeans(LogSummary* log_summary) {
	for (int i = 0; i < log_summary->n_sensors; i++) {
		log_summary->sensor_stats[i].mean = log_summary->sensor_stats[i].sum / log_summary->sensor_stats[i].count;
	}
}

void calculateLogStdDevs(LogSummary* log_summary) {
	for (int i = 0; i < log_summary->n_sensors; i++) {
		if (log_summary->sensor_stats[i].count <= 1) {
			// avoid division by zero; if only one sample, stddev is 0
			log_summary->sensor_stats[i].stddev = 0.0;
			continue;
		}
		// stddev = sqrt( (sumsq - sum^2)/(n*(n-1)) ) <-- calculate this second since we need mean and sumsq
		log_summary->sensor_stats[i].stddev = sqrt(((double)log_summary->sensor_stats[i].count * log_summary->sensor_stats[i].sum_squares - (double)log_summary->sensor_stats[i].sum * log_summary->sensor_stats[i].sum) / (double)(log_summary->sensor_stats[i].count * (log_summary->sensor_stats[i].count - 1)));
	}
}

int processLogData(FILE* input_stream, LogSummary* log_summary, int max_sensors) {
	// line length doesn't matter, we can process the file in smaller parts than line-by-line.
	//   until EOF:
	//     read from file for timestamp.
	//     read until whitespace (not newline)
	//      -> parse data, check extrema, increment sensor count 
	//      -> check n_sensor, check if not enough/too many sensors @ end of line.
	//     read until \n and repeat
	// Also - for first line only, determine number of sensors from number of successfully parsed entries.
	int sensor_id = 0;
	log_summary->n_sensors = 1;
	log_summary->max.value = -DBL_MAX;
	log_summary->min.value = DBL_MAX;
	int line = 0;
	char line_timestamp[20];
	double new_sensor_value = 0;
	while (true) {
		if (feof(input_stream)) {
			break;
		}

		// end if encounter a blank line (only leading whitespace then \n or EOF)
		fscanf_s(input_stream, "%*[ \t]");
		int c = filePeekNextChar(input_stream);
		if (c == '\n' || c == EOF) {
			break;
		}

		// read timestamp (skip leading ' ' or '\t')
		if (fscanf_s(input_stream, "%19[^ \t\n]", line_timestamp, 20) != 1) {
			fprintf(stderr, "Error while processing file: Unable to read timestamp on line %d.\n", line + 1);
			return -1;
		}

		line_timestamp[19] = '\0';

		// check timestamp length
		if (strlen(line_timestamp) != 19) {
			fprintf(stderr, "Error while processing file: Timestamp not correct length on line %d.\n", line + 1);
			return -1;
		}

		// read sensor values
		for (int sensor_id = 0; sensor_id < log_summary->n_sensors && sensor_id < max_sensors; sensor_id++) {
			// try to read new sensor value (skip leading ' ' or '\t'); if can't parse, therefore end of line or wrong character encountered
			if (fscanf_s(input_stream, "%*[ \t]%lf", &new_sensor_value) != 1) {
				// reached end of line or a wrong character
				// check next char without consuming it
				c = filePeekNextChar(input_stream);

				// if wrong character incountered:
				if (c != '\n' && c != EOF) {
					fprintf(stderr, "Error while processing file: Line %d contains an entry that couldn't be parsed.\n", line + 1);
					return -1;
				}

				// if not first line:
				if (line != 0) {
					fprintf(stderr, "Error while processing file: Line %d contains only %d data entries (%d required).\n", line + 1, sensor_id + 1, log_summary->n_sensors);
					return -1;
				}

				// if first line, then expect one fewer sensors/columns than this on subsequent lines:
				log_summary->n_sensors--;
				break;
			}

			// not yet end of line. if first line and there are allowed to be more sensors:
			if (line == 0 && log_summary->n_sensors < max_sensors) {
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
		while (true) {
			c = fgetc(input_stream);
			if (c == EOF || c == '\n') {
				break;
			}

			if (c != '\t' && c != ' ') {
				fprintf(stderr, "Error while processing file: Line %d contains more than %d data entries.\n", line + 1, log_summary->n_sensors);
				return -1;
			}
		}

		if (c != EOF) {
			line++;
		}
	}

	// handle case of no lines
	if (line == 0) {
		fprintf(stderr, "Error while processing file: No lines to process.\n");
		return -1;
	}

	return 0;
}

int outputStats(FILE* output_stream, const LogSummary log_summary) {
	// output min and max
	fprintf(output_stream, "Maximum recorded at %s (%f)\n", log_summary.max.timestamp, log_summary.max.value);
	fprintf(output_stream, "Minimum recorded at %s (%f)\n", log_summary.min.timestamp, log_summary.min.value);

	// output mean and stddev for each sensor. Example in lab instructions shows 2 decimal places.
	for (int sensor_id = 0; sensor_id < log_summary.n_sensors; sensor_id++) {
		fprintf(output_stream, "\nSensor %d:\n- mean: %.2f\n- deviation: %.2f\n", sensor_id + 1, log_summary.sensor_stats[sensor_id].mean, log_summary.sensor_stats[sensor_id].stddev);
	}

	return 0;
}