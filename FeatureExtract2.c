#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <omp.h>
#include <stdbool.h>
#include <assert.h>

typedef struct {
    double packet;
    double time;
} TimeSeries;

typedef struct {
    double RS;
    double size;
} DataPoint;


double calculateSubarrayMean(const TimeSeries* ts, int startIndex, int endIndex) {
    // values include [start, end)
    if (startIndex >= endIndex) {
        printf("%d, %d\n", startIndex, endIndex);
        printf("Wrong Start and End Indexes");
        exit(2);
    }
    double mean = 0;

    for (int i = startIndex; i < endIndex; ++i) {
        mean += ts[i].packet;
    }
    mean = mean / (endIndex - startIndex);
    return mean;
}

double calculateSlope(const DataPoint* datapoints, int start, int array_size) {
    double sumX = 0, sumY = 0, sumXY = 0, sumX2 = 0;
    for (int i = start; i < array_size; i++) {
        sumX += datapoints[i].size;
        sumY += datapoints[i].RS;
        sumXY += datapoints[i].size * datapoints[i].RS;
        sumX2 += datapoints[i].size * datapoints[i].size;
    }

    double numerator = (array_size - start) * sumXY - sumX * sumY;
    double denominator = (array_size - start) * sumX2 - sumX * sumX;

    if (denominator == 0) {
        fprintf(stderr, "Error: Denominator is zero, slope is undefined.\n");
        for (int i = start; i < array_size; i++) {
            printf("%lf", datapoints[i].size);
        }
        return 0; // Handling division by zero, could also exit the program
    }

    return numerator / denominator;
}

double calculateStandardDeviation(const TimeSeries datapoints[], int start, int array_size, double meanRS) {
    double sumSqDiff = 0;
    for (int i = start; i < array_size; i++) {
        double diff = datapoints[i].packet - meanRS;
        sumSqDiff += diff * diff;
    }

    return sqrt(sumSqDiff / (array_size - start));
}

double calculateRange(const TimeSeries datapoints[], int start_index, int end_index) {
    if (start_index > end_index) {
        fprintf(stderr, "Error: start_index cannot be greater than end_index.\n");
        return 0; // Return 0 or appropriate error code/value
    }

    double minRS = datapoints[start_index].packet;
    double maxRS = datapoints[start_index].packet;
    
    // Traverse the array from start_index to end_index
    for (int i = start_index; i < end_index; i++) {
        if (datapoints[i].packet < minRS) {
            minRS = datapoints[i].packet;
        }
        if (datapoints[i].packet > maxRS) {
            maxRS = datapoints[i].packet;
        }
    }

    return maxRS - minRS; // Return the range
}



bool checkFlawed(double value) {
    if (isnan(value)) {
        return true;
    } else if (isinf(value)) {
        return true;
    } else {
        return false;
    }
}

// Function to read CSV file and store it in a TimeSeries array
int readCSV(const char* file_time_name, const char* file_packet_name, TimeSeries* ts, int max_size) {
    FILE *file_time = fopen(file_time_name, "r");
    FILE *file_packet = fopen(file_packet_name, "r");
    if (!file_time || !file_packet) {
        printf("File opening failed.\n");
        exit(1);
    }
    
    int time_counter = 0;
    for (int i = 0; i < max_size; i++) {
        TimeSeries placeholder;
        int counter1  = fscanf(file_time, "%lf", &placeholder.time);
        int counter2 = fscanf(file_packet, "%lf", &placeholder.packet);
        int result = placeholder.time/0.01;
        if (result == time_counter) {
            ts[time_counter].packet += placeholder.packet;
        }
        else{
            time_counter = result;
            ts[time_counter] = placeholder;
        }

        if (counter1 != 1 || counter2 != 1) {
            printf("Terminating at %d\n", i);
            fclose(file_time);
            fclose(file_packet);
            return i;
        }
    }
    max_size = time_counter;

    fclose(file_time);
    fclose(file_packet);
    return max_size;
}



void writeToCSV(double* hursts, int size, const char* filename) {
    FILE *file = fopen(filename, "w");

    for (int i = 0; i < size * 3; i++) {
        fprintf(file, "%.2f", hursts[i]);  // Writing the double value with two decimal places
        if (i % 3 == 2) {  // After every third element, start a new line
            fprintf(file, "\n");
        } else if (i != size - 1) {  // Place a comma if it's not the last element in a row or the last element overall
            fprintf(file, ",");
        }
    }

    fclose(file);
}

double calculateHurst(const TimeSeries* ts, int segment_size, int location, int array_size) {
    int i = location;
    int count = 0;
    int endTime = segment_size + ts[location].time;
    while (i < array_size) {
        if (ts[location + i].time > endTime) {
            break;
        }
        else {
            ++count;
        }
        ++i;
    }
    int L = count / 4;
    if (count <= 10) return 0;
    // k is the length of each individual segment
    DataPoint dataset[L - 4];
    for (int k = 4; k < L; ++k) {
        int num_segments = count / k;
        double* means= malloc(num_segments * sizeof(double));
        double* stds = malloc(num_segments * sizeof(double));
        double* ranges = malloc(num_segments * sizeof(double));

        for (int j = 0; j < num_segments; ++j) {
            means[j] = calculateSubarrayMean(ts, j * k, (j + 1)* k);
            stds[j] = calculateStandardDeviation(ts, j * k, (j + 1) * k, means[j]);
            ranges[j] = calculateRange(ts, j * k, (j + 1)* k);
        }

        double ratios = 0;
        int nonzero_counter = 0;
        for (int j = 0; j < num_segments; ++j) {
            if (stds[j] <= 0) {
                continue;
            }
            ratios += ranges[j] / stds[j];
            ++nonzero_counter;
        }
        double RS = ratios / nonzero_counter;
        dataset[k - 4].RS = log10(RS);
        dataset[k - 4].size = log10(k);
    }
    double hurst = calculateSlope(dataset, 0, L - 4);
    if (hurst < 0 | hurst > 1 | checkFlawed(hurst)) {
        printf("%lf", hurst);
        exit(1);
    }
    return hurst;
}


int main() {
    int segment_sizes[3] = {10, 30, 60};
    // Allocating memory for a very large array
    TimeSeries* ts = malloc(50331648 * sizeof(TimeSeries)); // Corrected to sizeof(TimeSeries)
    if (ts == NULL) {
        printf("Memory allocation failed.\n");
        return 1; // Return an error code indicating failure
    }
   
    int size = readCSV("Name_Time_VideoInjection_tester", "Name_Packet_VideoInjection_tester", ts, 50331648); // Correct filename extension and variable name

    double* hursts = malloc(3 * size * sizeof(double));
    if (hursts == NULL) {
        printf("Memory allocation failed.\n");
        return 1; // Return an error code indicating failure
    }
    int i;
    int j;
    #pragma omp parallel for num_threads(4)
    for (i = 0; i < size; i++) {
        for (j = 0; j < 3; j++) {
            hursts[i * 3 + j] = calculateHurst(ts, segment_sizes[j], i, size);  // Assign a value to each element
            
        }
        if (i % 1 == 0) {
        printf("DONE CALCULATING %d out of %d, %d, %lf \n", i, size, omp_get_thread_num(), hursts[i]);
        }
    }
    writeToCSV(hursts, size, "outputs.csv");
    free(ts); // Free allocated memory
    free(hursts);
    return 0;
}