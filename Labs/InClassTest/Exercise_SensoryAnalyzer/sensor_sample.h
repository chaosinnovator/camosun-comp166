/*
* sensor_sample.h
* 
* Contains structure definition and function prototypes for sensor
* sample parsing, analysis, and output.
* 
*/

typedef struct {
	int timestamp_ms;
	int vibration;
	double temperature_c;
	char status;
} SensorSample;

int readEntireCsv(const char* filename, SensorSample* sample_array, int n_max_samples);

int minVibration(const SensorSample* sample_array, int n_samples);
int maxVibration(const SensorSample* sample_array, int n_samples);
int maxVibrationIndex(const SensorSample* sample_array, int n_samples);
void outputAverageVibration(const SensorSample* sample_array, int n_samples);

double averageAbnormalReadings(const SensorSample* sample_array, int n_samples); //status == 'W' or 'A'
int countAlarms(const SensorSample* sample_array, int n_samples); // status == 'A'
int countVibrationExceedingThreshold(const SensorSample* sample_array, int n_samples, int threshold);
double maxTemperatureIndex(const SensorSample* sample_array, int n_samples);

void sortByVibrationDescending(SensorSample* sample_array, int n_samples);
void printSamples(const SensorSample* sample_array, int n_samples);