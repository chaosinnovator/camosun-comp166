/*
* log_summary.h
* 2025F COMP-166 Lab 5 - Thomas Boland - C0556991
* 
* Contains structures for holding log summary and statistics data and
* function prototypes for processing and manipulating log data.
*
*/

#include <stdio.h>

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

/*
* @brief Calculates the mean for each sensor in the provided LogSummary structure.
* @param log_summary Pointer to LogSummary structure containing sensor data.
*/
void calculateLogMeans(LogSummary* log_summary);

/*
* @brief Calculates the standard deviation for each sensor in the provided LogSummary structure.
* @param log_summary Pointer to LogSummary structure containing sensor data.
*/
void calculateLogStdDevs(LogSummary* log_summary);

/*
* @brief Processes log data from the specified file and populates the provided LogSummary structure.
* @param input_stream File/input stream to read log data from.
* @param log_summary Pointer to LogSummary structure to populate with processed data. 
* @param max_sensors Maximum number of sensors expected in the log data.
* @return Returns 0 if successful, non-zero if an error occurred.
*/
int processLogData(FILE* input_stream, LogSummary* log_summary, int max_sensors);

/*
* @brief Outputs the statistics contained in the provided LogSummary structure to the specified stream.
* @param stream File/output stream to write statistics to.
* @param log_summary LogSummary structure containing statistics to output.
* @return Returns 0 if successful, non-zero if an error occurred.
*/
int outputStats(FILE* output_stream, const LogSummary log_summary);