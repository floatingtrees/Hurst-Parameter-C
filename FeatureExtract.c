#include <stdio.h>
#include <stdlib.h>
#include <math.h>

typedef struct {
    float packet;
    float time;
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
        int counter1  = fscanf(file_time, "%f", &ts[i].time);
        int counter2 = fscanf(file_packet, "%f", &ts[i].packet);
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

float calculateHurst(TimeSeries* ts, int segment_size, int location, int array_size) {
    int i = location;
    int endTime = segment_size + ts[location].time;
    int count = 0;
    while (i < array_size) {
        if (ts[location + i].time < endTime) {
            break;
        }
        else {
            
            ++count;
        }
    }
    
    int L = count / 4; // automatically casts to integer division
    float* differences = malloc(4 * L * sizeof(float));
    float means[4];
    float stds[4];
    float cumulative_deviations[4];
    float highest_numbers[4];
    float lowest_numbers[4];
    int j = 0;

    for (i = 0; i < 4; ++i){ // calculate the mean 
        means[i] = 0;
        for (j = 0; j < L; ++j) {
            means[i] += ts[location + i].packet;
        }
        means[i] = means[i] / L;
    }
    int index = 0;
    
    for (i = 0; i < 4; ++i) { // calculate ranges, std, cumsum
        int std_counter = 0;
        float cumsum = 0;
        
        float highest = ts[index].packet;
        float lowest = ts[index].packet;

        for (j = 0; j < L; ++j) {
            if (ts[index].packet < lowest) {
                lowest = ts[index].packet;
            }
            else if (ts[index].packet > highest) {
                highest = ts[index].packet;
            }
            differences[index] = ts[index].packet - means[i];
            cumsum +=ts[index].packet - means[i];
            std_counter = std_counter + differences[index] * differences[index];
            ++index;
        }
        cumulative_deviations[i] = cumsum;
        std_counter = std_counter / L;
        stds[i] = sqrt(std_counter);
        highest_numbers[i] = highest;
        lowest_numbers[i] = lowest;
    }

    return 0.1;
}

void writeToCSV(float* hursts, int size, const char* filename) {
    FILE *file = fopen(filename, "w");

    for (int i = 0; i < size * 3; i++) {
        fprintf(file, "%.2f", hursts[i]);  // Writing the float value with two decimal places
        if (i % 3 == 2) {  // After every third element, start a new line
            fprintf(file, "\n");
        } else if (i != size - 1) {  // Place a comma if it's not the last element in a row or the last element overall
            fprintf(file, ",");
        }
    }

    fclose(file);
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
    
    float* hursts = malloc(3 * size * sizeof(float));
    if (hursts == NULL) {
        printf("Memory allocation failed.\n");
        return 1; // Return an error code indicating failure
    }

    for (int i = 0; i < size; i++) {
        for (int j = 0; j < 3; j++) {
            // Accessing the element at row i, column j
            hursts[i * 3 + j] = calculateHurst(ts, segment_sizes[j], i, size);  // Assign a value to each element
        }
    }

    writeToCSV(hursts, size, "outputs.csv");
    free(ts); // Free allocated memory
    free(hursts);
    return 0;
}