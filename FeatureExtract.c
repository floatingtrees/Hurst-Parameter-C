// Compile with
// gcc -fopenmp -o FeatureExtract FeatureExtract.c
// Uses openmp for parallelism
// Ran into many nan values for small segments due to a range and std of 0, so epsilons and other heuristics were used
// CSVs are expected to have a newline between each entry (no commas)
// If they are seperated by commas, run newline_maker.py on the file to fix it

// When using for your own code, replace "Name_Time_VideoInjection_tester" with your csv file containing timestamps
// And replace "Name_Packet_VideoInjection_tester" with your csv containing packet sizes
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <omp.h>
#include <stdbool.h>
#include <assert.h>

typedef struct
{
    double packet;
    double time;
} TimeSeries;

typedef struct
{
    double RS;
    double size;
} DataPoint;

// calculates the mean of a segment
double calculateSubarrayMean(const TimeSeries *ts, int startIndex, int endIndex)
{
    // values include [start, end)
    if (startIndex >= endIndex)
    {
        printf("%d, %d\n", startIndex, endIndex);
        printf("Wrong Start and End Indexes");
        exit(2);
    }
    double mean = 0;

    for (int i = startIndex; i < endIndex; ++i)
    {
        mean += ts[i].packet;
    }
    mean = mean / (endIndex - startIndex);
    return mean;
}

// calculates the line of best fit for a set of points
double calculateSlope(const DataPoint *datapoints, int start, int array_size)
{
    double sumX = 0, sumY = 0, sumXY = 0, sumX2 = 0;
    for (int i = start; i < array_size; i++)
    {
        sumX += datapoints[i].size;
        sumY += datapoints[i].RS;
        sumXY += datapoints[i].size * datapoints[i].RS;
        sumX2 += datapoints[i].size * datapoints[i].size;
    }

    double numerator = (array_size - start) * sumXY - sumX * sumY;
    double denominator = (array_size - start) * sumX2 - sumX * sumX;

    if (denominator == 0)
    {
        for (int i = start; i < array_size; i++)
        {
            printf("%lf", datapoints[i].size);
        }
        fprintf(stderr, "Error: Denominator is zero, slope is undefined.\n");
        exit(1);
        return 0; // Handling division by zero, could also exit the program
    }

    return numerator / denominator;
}
// calculates the standard deviation of an array subset
double calculateStandardDeviation(const TimeSeries datapoints[], int start, int array_size, double meanRS)
{
    double sumSqDiff = 0;
    for (int i = start; i < array_size; i++)
    {
        double diff = datapoints[i].packet - meanRS;
        sumSqDiff += diff * diff;
    }

    return sqrt(sumSqDiff / (array_size - start));
}

// Calculates the range of a segment
double calculateRange(const TimeSeries datapoints[], int start_index, int end_index)
{
    if (start_index > end_index)
    {
        fprintf(stderr, "Error: start_index cannot be greater than end_index.\n");
        return 0;
    }

    double minRS = datapoints[start_index].packet;
    double maxRS = datapoints[start_index].packet;

    for (int i = start_index; i < end_index; i++)
    {
        if (datapoints[i].packet < minRS)
        {
            minRS = datapoints[i].packet;
        }
        if (datapoints[i].packet > maxRS)
        {
            maxRS = datapoints[i].packet;
        }
    }

    return maxRS - minRS; // Return the range
}

// Debugging function to check for nans and infinities
bool checkFlawed(double value)
{
    if (isnan(value))
    {
        return true;
    }
    else if (isinf(value))
    {
        return true;
    }
    else
    {
        return false;
    }
}

// Function to read CSV file and store it in a TimeSeries array
// One CSV should contain timestamps, and the second should contain packet sizes
int readCSV(const char *file_time_name, const char *file_packet_name, TimeSeries *ts, int max_size)
{
    FILE *file_time = fopen(file_time_name, "r");
    FILE *file_packet = fopen(file_packet_name, "r");
    if (!file_time || !file_packet)
    {
        printf("File opening failed.\n");
        exit(1);
    }

    int time_counter = 0;
    for (int i = 0; i < max_size; i++)
    {
        TimeSeries placeholder;
        int counter1 = fscanf(file_time, "%lf", &placeholder.time);
        int counter2 = fscanf(file_packet, "%lf", &placeholder.packet);

        int result = placeholder.time / 0.01;
        if (counter1 != 1 || counter2 != 1)
        {

            printf("Terminating at %d\n", time_counter);
            fclose(file_time);
            fclose(file_packet);
            return time_counter;
        }
        if (result == time_counter)
        {
            ts[time_counter].packet += placeholder.packet;
        }
        else
        {
            placeholder.time = floor(placeholder.time * 100) / 100.0;
            time_counter = result;
            ts[time_counter] = placeholder;
        }
    }
    max_size = time_counter;

    fclose(file_time);
    fclose(file_packet);
    return time_counter;
}

// writes results to the output CSV files
void writeToCSV(double *hursts, int size, const char *filename)
{
    FILE *file = fopen(filename, "w");

    for (int i = 0; i < size * 3; i++)
    {
        fprintf(file, "%.5f", hursts[i]);
        if (i % 3 == 2)
        { // After every third element, start a new line
            fprintf(file, "\n");
        }
        else if (i != size - 1)
        { // Place a comma if it's not the last element in a row or the last element overall
            fprintf(file, ",");
        }
    }

    fclose(file);
}

// Debug function
void CSVDebug(TimeSeries *ts, int size, const char *filename)
{
    FILE *file = fopen(filename, "w");
    for (int i = 0; i < size; i++)
    {
        fprintf(file, "%.2f, %.2f", ts[i].packet, ts[i].time); // Writing the double value with two decimal places
        fprintf(file, "\n");
    }
}

// the function that combines previous function calls to calculate hurst
double calculateHurst(const TimeSeries *ts, const double segment_size, const int location, const int array_size)
{
    int i = location;
    int count = 0;
    double endTime = segment_size + ts[location].time;
    while (i < array_size)
    {
        if (ts[i].time > endTime)
        {
            break;
        }
        else
        {
            ++count;
        }
        ++i;
    }

    int L = count / 4;
    if (L <= 6)
        return -1;
    // k is the length of each individual segment
    DataPoint dataset[L - 4];
    for (int k = 4; k < L; ++k)
    {
        int num_segments = count / k;
        double *means = malloc(num_segments * sizeof(double));
        double *stds = malloc(num_segments * sizeof(double));
        double *ranges = malloc(num_segments * sizeof(double));

        for (int j = 0; j < num_segments; ++j)
        {
            means[j] = calculateSubarrayMean(ts, location + j * k, location + (j + 1) * k);
            stds[j] = calculateStandardDeviation(ts, location + j * k, location + (j + 1) * k, means[j]);
            ranges[j] = calculateRange(ts, location + j * k, location + (j + 1) * k);
        }

        double ratios = 0;
        int nonzero_counter = 0;
        for (int j = 0; j < num_segments; ++j)
        {
            if (stds[j] <= 0)
            {
                continue;
            }
            ratios += ranges[j] / stds[j];
            ++nonzero_counter;
        }
        double RS = ratios / nonzero_counter;
        dataset[k - 4].RS = log10(RS);
        dataset[k - 4].size = log10(k);
        free(means);
        free(stds);
        free(ranges);
    }
    double hurst = calculateSlope(dataset, 0, L - 4);
    if (hurst <= 0.0001 || hurst > 1 || checkFlawed(hurst))
    {
        printf("%lf", hurst);
    }
    return hurst;
}

int main()
{
    double segment_sizes[3] = {10.0, 30.0, 60.0};
    // Allocating memory for a very large array
    // The 50331648 is arbitrary, but it's the maximum theoretical size of the CSV file I used
    // Future people using this code, make sure you don't overrun your buffer
    TimeSeries *ts = malloc(50331648 * sizeof(TimeSeries));
    for (int i = 0; i < 50331648; ++i)
    {
        ts[i].packet = 0;
        ts[i].time = i * 0.01;
    }
    if (ts == NULL)
    {
        printf("Memory allocation failed.\n");
        return 1; // Return an error code indicating failure
    }

    int size = readCSV("Name_Time_VideoInjection_tester", "Name_Packet_VideoInjection_tester", ts, 50331648); // Use the correct number here too
    double *hursts = malloc(3 * size * sizeof(double));
    if (hursts == NULL)
    {
        printf("Memory allocation failed.\n");
        return 1;
    }
    int i;
    int j;
#pragma omp parallel for private(j)
    for (i = 0; i < size; i++)
    {
        for (j = 0; j < 3; j++)
        {
            hursts[i * 3 + j] = calculateHurst(ts, segment_sizes[j], i, size); // Assign a value to each element
        }
        if (i % 1000 == 0)
        {
            printf("DONE CALCULATING %d out of %d, %d, %lf \n", i, size, omp_get_thread_num(), hursts[i * 3]);
        }
    }
    writeToCSV(hursts, size, "outputs.csv");
    free(ts); // Free allocated memory
    free(hursts);
    return 0;
}