/*
* sensor_sample.c
*/

#include <stdio.h>
#include <stdlib.h>
#include "sensor_sample.h"

int readEntireCsv(const char* filename, SensorSample* sample_array, int n_max_samples) {
	FILE* fptr = fopen(filename, "r");
	if (fptr == NULL) {
		perror("Failed to open file");
		return 0;
	}

	// skip first header line
	fscanf_s(fptr, "%*[^\n]");

	int sample_count = 0;
	// read in sensor lines
		// parse into struct & store in array
	int matches;
	for (; sample_count < n_max_samples && !feof(fptr); sample_count++) {
		SensorSample new_sample;
		new_sample.temperature_c = 0;
		if ((matches = fscanf_s(fptr, " %d,%d,%lf,%c",
			&new_sample.timestamp_ms,
			&new_sample.vibration,
			&new_sample.temperature_c,
			&new_sample.status, 1
		)) != 4) {
			if (matches > 0) {
				fprintf(stderr, "Error parsing line %d\n", sample_count + 2); // +2 for header and 0-index
			}
			break; // stop reading on parse error
		}
		sample_array[sample_count] = new_sample;
	}

	fclose(fptr);
	return sample_count;
}


int minVibration(const SensorSample* sample_array, int n_samples) {
	if (n_samples <= 0) {
		return 0;
	}
	int min_vibration = sample_array[0].vibration;
	for (int i = 1; i < n_samples; i++) {
		if (sample_array[i].vibration < min_vibration) {
			min_vibration = sample_array[i].vibration;
		}
	}
	return min_vibration;
}

int maxVibration(const SensorSample* sample_array, int n_samples) {
	if (n_samples <= 0) {
		return 0;
	}
	int max_vibration = sample_array[0].vibration;
	for (int i = 1; i < n_samples; i++) {
		if (sample_array[i].vibration > max_vibration) {
			max_vibration = sample_array[i].vibration;
		}
	}
	return max_vibration;
}

int maxVibrationIndex(const SensorSample* sample_array, int n_samples) {
	if (n_samples <= 0) {
		return -1;
	}
	int max_idx = 0;
	for (int i = 1; i < n_samples; i++) {
		if (sample_array[i].vibration > sample_array[max_idx].vibration) {
			max_idx = i;
		}
	}
	return max_idx;
}
void outputAverageVibration(const SensorSample* sample_array, int n_samples) {
	// compute as an in-place average
	double avg = 0;
	for (int i = 0; i < n_samples; i++) {
		avg = (avg * i + sample_array[i].vibration) / (i + 1);
	}

	printf("Average value: %f", avg);
}

double averageAbnormalReadings(const SensorSample* sample_array, int n_samples) {
	// status != 'N'
	// compute as an in-place average
	double avg = 0;
	for (int i = 0; i < n_samples; i++) {
		if (sample_array[i].status == 'N') { // skip normal readings
			continue;
		}

		avg = (avg * i + sample_array[i].vibration) / (i + 1);
	}

	return avg;
}

int countAlarms(const SensorSample* sample_array, int n_samples) {
	// status == 'A'
	int count = 0;
	for (int i = 0; i < n_samples; i++) {
		if (sample_array[i].status != 'A') { // skip non-alarm readings
			continue;
		}

		++count;
	}
}

int countVibrationExceedingThreshold(const SensorSample* sample_array, int n_samples, int threshold) {
	int count = 0;
	for (int i = 0; i < n_samples; i++) {
		if (sample_array[i].status <= threshold) {
			continue;
		}

		++count;
	}
}

double maxTemperatureIndex(const SensorSample* sample_array, int n_samples) {
	if (n_samples <= 0) {
		return -1;
	}
	int max_idx = 0;
	for (int i = 1; i < n_samples; i++) {
		if (sample_array[i].temperature_c > sample_array[max_idx].temperature_c) {
			max_idx = i;
		}
	}
	return max_idx;
}

void sortByVibrationDescending(SensorSample* sample_array, int n_samples) {
	// bubble sort by sample_array[i].vibration descending
	// already handles arrays of length < 2.
	// for each position from start to last unsorted-1, if position < position+1 then swap
	int tmp;
	for (int last_unsorted = n_samples - 1; last_unsorted > 0; last_unsorted--) {
		for (int i = 0; i < last_unsorted; i++) {
			if (sample_array[i].vibration >= sample_array[i + 1].vibration) {
				// skip
				continue;
			}
			// swap
			tmp = sample_array[i].vibration;
			sample_array[i].vibration = sample_array[i + 1].vibration;
			sample_array[i + 1].vibration = tmp;
		}
	}
}

void printSamples(const SensorSample* sample_array, int n_samples) {
	printf("Time (ms) | Vibration | Temp (C) | Status\n");
	for (int i = 0; i < n_samples; i++) {
		printf("% 9d | % 9d | %8.3f | %c\n",
			sample_array[i].timestamp_ms,
			sample_array[i].vibration,
			sample_array[i].temperature_c,
			sample_array[i].status
		);
	}
}