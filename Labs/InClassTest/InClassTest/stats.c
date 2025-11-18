#include "stats.h"

int mean(int count, double* values, double* result) {
	// must have at least one element.
	if (count < 1) {
		return -1;
	}

	(*result) = 0;
	for (int i = 0; i < count; i++) {
		(*result) = ((*result) * i + values[i]) / (i + 1);
	}

	return 0;
}