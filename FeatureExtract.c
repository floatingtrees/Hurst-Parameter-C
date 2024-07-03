#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <omp.h>


typedef struct {
    double packet;
    double time;
} TimeSeries;

typedef struct {
    double RS;
    double size;
} DataPoint;

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
            printf("Terminating at %d\n", i);
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

double calculateHurst(TimeSeries* ts, int segment_size, int location, int array_size) {
    
    int i = location;
    int endTime = segment_size + ts[location].time;
    int count = 0;
    
    while (i < array_size) {
        if (ts[location + i].time > endTime) {
            break;
        }
        else {
            ++count;
        }
        ++i;
    }
    
    
    const int L = count / 4; // automatically casts to integer division

    if (L < 4) return -1; // deal with this edge case later

    DataPoint dataset[L - 3];
    for (int k = 4; k < L; ++k) {
        int L2 = L / k;
        double means[k];
        double stds[k];
        double cumulative_deviations[k];
        double ranges[k];
        int j = 0;

        for (i = 0; i < k; ++i){ // calculate the mean 
            means[i] = 0;
            for (j = 0; j < L2; ++j) {
                means[i] += ts[location + j].packet;
            }
            means[i] = means[i] / (double)L2;
        }
        int index = 0;
        double mean_centered_value;
        
        for (i = 0; i < k; ++i) { // calculate ranges, std, cumsum
        // CHECK IF RANGE IS CALCULATED CORRECTLY
            double std_counter = 0;
            double cumsum = 0;

            double highest = ts[index].packet - means[i];
            double lowest = ts[index].packet - means[i];
            double centered_value;
            for (j = 0; j < L2; ++j) {
                centered_value = ts[index].packet - means[i];
                if (centered_value< lowest) {
                    lowest = centered_value;
                }
                else if (centered_value > highest) {
                    highest = centered_value;
                }
                mean_centered_value = centered_value;
                cumsum += mean_centered_value;
                std_counter +=  mean_centered_value * mean_centered_value;
                ++index;
                //printf("%lf, %lf, %lf\n", lowest, highest, means[i]);
            }
            
            cumulative_deviations[i] = cumsum;
            std_counter = std_counter / L2;
            stds[i] = sqrt(std_counter);
            ranges[i] = highest - lowest;
            //printf("%lf, %lf, %lf, %d\n", highest, lowest, stds[i], L2);
        }
        double ratios = 0;
        int counter2 = 0;
        for (i = 0; i < k; ++i) {
            if (stds[i] < 0.0001 || ranges[i] < 0.0001) {
                printf("%lf, %lf, %lf\n", log10(ranges[i] / stds[i]), ranges[i], stds[i]);
                continue;
                
            }
            ratios += log10(ranges[i] / stds[i]);
            
            counter2++;
        }
        
        // take the mean of ratios
        dataset[k - 3].RS = ratios / (double)counter2;
        dataset[k-3].size = log10((double)counter2);
        //printf("\n%lf, %d, %lf", ratios, counter2, log10(counter2));
    }

    int n = L - 4;
    double sum_xy = 0;
    double sum_x = 0;
    double sum_y = 0;
    double sum_x_squared = 0;
    for (i = 0; i < n; ++i) {
        sum_xy += dataset[i].RS * dataset[i].size;
        sum_x += dataset[i].size;
        sum_y += dataset[i].RS;
        sum_x_squared += dataset[i].size * dataset[i].size;
    }
    double hurst = (n * sum_xy - (sum_x * sum_y)) / (n * sum_x_squared - (sum_x * sum_x));

    // sum_y doesn't work
    printf("\n%lf:%lf:%lf:%lf\n", sum_xy, sum_x, sum_y, sum_x_squared);
    return hurst;
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
    //#pragma omp parallel for
    for (int i = 0; i < size; i++) {
        for (int j = 0; j < 3; j++) {
            // Accessing the element at row i, column j
            // CHANGE THE PLUS ONE AFTER DEBUGGING
            // VERY VERY IMPORTANT
            hursts[i * 3 + j] = calculateHurst(ts, segment_sizes[j+1], i, size);  // Assign a value to each element
            
        }
        if (i % 1 == 0) {
        printf("DONE CALCULATING %d out of %d, %d, %d \n", i, size, omp_get_thread_num(), hursts[i]);
        return 1;
        }
    }
    printf("HIHIHI");
    writeToCSV(hursts, size, "outputs2.csv");
    free(ts); // Free allocated memory
    free(hursts);
    return 0;
}