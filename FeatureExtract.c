#include <stdio.h>
#include <stdlib.h>

typedef struct {
    double packet;
    double time;
} TimeSeries;

// Function to read CSV file and store it in a TimeSeries array
int readCSV(const char* file_time_name, const char* file_packet_name, TimeSeries* ts, int max_size) {
    FILE *file_time = fopen(file_time_name, "r");
    FILE *file_packet = fopen(file_packet_name, "r");
    if (!file_time || !file_packet) {
        printf("File opening failed.\n");
        exit(1);
    }

    for (int i = 0; i < max_size; i++) {
        int counter1  = fscanf(file_time, "%lf", &ts[i].time);
        int counter2 = fscanf(file_packet, "%lf", &ts[i].packet);
        if (counter1 != 1 || counter2 != 1) {
            printf("Terminating at %d", i);
            fclose(file_time);
            fclose(file_packet);
            return i;
        }
    }
    

    fclose(file_time);
    fclose(file_packet);
    return max_size;
}

// Main computation function (simplified version)
void processTimeSeries(TimeSeries* ts, int size) {
    for (int i = 0; i < size; i++) {
        printf("Time: %lf, Packet: %lf\n", ts[i].time, ts[i].packet);
    }
}

double calculateHurst(TimeSeries* ts, int segment_size) {
    return 0.1;
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

    for (int i = 0; i < size; i++) {
        for (int j = 0; j < 3; j++) {
            // Accessing the element at row i, column j
            hursts[i * 3 + j] = calculateHurst(ts, segment_sizes[j]);  // Assign a value to each element
        }
    }

    free(ts); // Free allocated memory
    free(hursts);
    return 0;
}