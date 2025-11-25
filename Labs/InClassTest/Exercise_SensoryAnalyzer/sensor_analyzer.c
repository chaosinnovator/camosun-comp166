/*
* sensor_analyzer.c
* 2025F COMP166 - In-class example from Nov 21, 2025 - Thomas Boland
*/

#include <stdio.h>
#include <stdlib.h>
#include "sensor_sample.h"

// could store SensorSample in an array and resize it with realloc
// alternatively, could use a linked list.
// in-class example just assumes a fixed length MAX_SAMPLES
#define MAX_SAMPLES 500

int main(int argc, char* argv[]) {

	// read samples from CSV file. unknown number of lines.
	// first line is a header
	// status: N (normal), W (warn), A (alarm)
	// example:
	//  timestamp_ms,vibration,temperature_c,status
	//  0,10,35.0,N
	//  250,15,36.2,W

	SensorSample* samples[MAX_SAMPLES];
	int n_samples = readEntireCsv(argv[1], samples, MAX_SAMPLES);

	if (n_samples <= 0) {
		fprintf(stderr, "No samples read from file %s\n", argv[1]);
		return EXIT_FAILURE;
	}

	sortByVibrationDescending(samples, n_samples);

	printSamples(samples, n_samples);

	return EXIT_SUCCESS;
}