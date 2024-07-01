#include <stdio.h>
#include <stdlib.h>

typedef struct {
    double packet;
    double time;
} TimeSeries;

// Function to read CSV file and store it in a TimeSeries array
void readCSV(const char* file_time_name, const char* file_packet_name, TimeSeries* ts, int max_size) {
    FILE *file_time = fopen(file_time_name, "r");
    FILE *file_packet = fopen(file_packet_name, "r");
    if (!file_time || !file_packet) {
        printf("File opening failed.\n");
        return;
    }

    for (int i = 0; i < max_size; i++) {
        fscanf(file_time, "%lf", &ts[i].time);
        fscanf(file_packet, "%lf", &ts[i].packet);
    }

    fclose(file_time);
    fclose(file_packet);
}

// Main computation function (simplified version)
void processTimeSeries(TimeSeries* ts, int size) {
    for (int i = 0; i < size; i++) {
        printf("Time: %lf, Packet: %lf\n", ts[i].time, ts[i].packet);
    }
}

int main() {
    // Allocating memory for a very large array
    TimeSeries* ts = malloc(50331648 * sizeof(TimeSeries)); // Corrected to sizeof(TimeSeries)
    if (ts == NULL) {
        printf("Memory allocation failed.\n");
        return 1; // Return an error code indicating failure
    }

    readCSV("Name_Time_VideoInjection", "Name_Packet_VideoInjection", ts, 50331648); // Correct filename extension and variable name
    processTimeSeries(ts, 50331648);

    free(ts); // Free allocated memory
    return 0;
}