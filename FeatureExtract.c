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

double calculateHurst(const TimeSeries* ts, int segment_size, int location, int array_size) {
    
    int i = location; // location represents start location
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
    //printf("COUNT IS %d\n", count);
    
    const int L = count / 4; // automatically casts to integer division
    if (L < 4) return -1; // deal with this edge case later

    DataPoint dataset[L]; // first few points are filler

    for (int k = 4; k < L; ++k) { // k = num_segments
        const int L2 = count / k;
        double means[k];
        double stds[k];
        double ranges[k];
        int j = 0;
        // if k == 4, divide array into 4 segments
        for (i = 0; i < k; ++i){ // calculate the mean 
            means[i] = calculateSubarrayMean(ts, i * L2, (i+1) * L2);
        }
        int index = 0;
        double mean_centered_value;
        
        for (i = 0; i < k; ++i) { // calculate ranges, std
        // CHECK IF RANGE IS CALCULATED CORRECTLY
            double std_counter = 0;

            double highest = ts[index].packet - means[i];
            double lowest = ts[index].packet - means[i];
            double centered_value;
            for (j = 0; j < L2; ++j) {
                centered_value = ts[index].packet - means[i];
                if (centered_value < lowest) {
                    lowest = centered_value;
                }
                else if (centered_value > highest) {
                    highest = centered_value;
                }
                mean_centered_value = centered_value;
                std_counter = std_counter +  (mean_centered_value * mean_centered_value);
                ++index;
                // INDEX INCREMENTORS IS WORKING
                //printf("%d, %d, %d, %d\n", index, j + i * L2 + 1, i, k);
                assert(index == j + i * L2 + 1);
                assert(index <= array_size);
                //printf("%lf, %lf, %lf\n", lowest, highest, means[i]);
            }
            
            std_counter = std_counter / L2;
            stds[i] = sqrt(std_counter);
            ranges[i] = highest - lowest;
            //printf("%lf, %lf, %lf, %d\n", highest, lowest, stds[i], L2);
        }
        double ratios = 0;
        int counter2 = 0;
        for (i = 0; i < k; ++i) {
            if (stds[i] < 0.000001 || ranges[i] < 0.000001) {
                continue;
            }
            ratios += ranges[i] / stds[i];
            // std consistently lower than range
            counter2++;
        }
        //printf("%lf", ratios / (double)counter2);
        // take the mean of ratios
        dataset[k].RS = log10(ratios / (double)counter2);
        dataset[k].size = log10((double) L2); // It's either k or L2
        if (checkFlawed(dataset[k].RS) || checkFlawed(dataset[k].size)) {
            printf("\n%lf, %lf,  %d, %d", dataset[k].RS, dataset[k].size, k, counter2);
            exit(1);
        }

    }

    int n = L - 3;
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
    if (checkFlawed(hurst) | hurst > 1) {
        printf("%lf, %lf, %lf, %lf, %d", sum_xy, sum_x, sum_y, sum_x_squared, n);
        for (int v = 0; v < n; v = v + 1) {
            printf("%lf, %lf\n", dataset[v].RS, dataset[v].size);
        }
        printf("Hyperparameters: %d, %d, %lf", n, count, hurst);
        exit(1);
    }
    // sum_y doesn't work 
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
            hursts[i * 3 + j] = calculateHurst(ts, segment_sizes[j], i, size);  // Assign a value to each element
            
        }
        if (i % 1 == 0) {
        printf("DONE CALCULATING %d out of %d, %d, %lf \n", i, size, omp_get_thread_num(), hursts[i]);
        }
    }
    printf("HIHIHI");
    writeToCSV(hursts, size, "outputs2.csv");
    free(ts); // Free allocated memory
    free(hursts);
    return 0;
}