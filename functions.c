#include <stdio.h>
#include <stdlib.h>
#include <math.h>

typedef struct {
    double packet;
    double time;
} TimeSeries;

typedef struct {
    double RS;
    double size;
} DataPoint;

double calculateSubarrayMean(TimeSeries* ts, int startIndex, int endIndex) {
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

double calculateSlope(DataPoint* datapoints, int start, int array_size) {
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
        return 0; // Handling division by zero, could also exit the program
    }

    return numerator / denominator;
}

double calculateStandardDeviation(TimeSeries datapoints[], int start, int array_size, double meanRS) {
    double sumSqDiff = 0;
    for (int i = start; i < array_size; i++) {
        double diff = datapoints[i].packet - meanRS;
        sumSqDiff += diff * diff;
    }

    return sqrt(sumSqDiff / (array_size - start));
}

double calculateRange(TimeSeries datapoints[], int start_index, int end_index) {
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

int main() {
     TimeSeries tsArray[5] = {
        {1.0, 1.1},
        {2.0, 2.2},
        {3.0, 3.3},
        {4.0, 4.4},
        {5.0, 5.5}
    };

    double mean = calculateSubarrayMean(tsArray, 1, 5);
    double std = calculateStandardDeviation(tsArray, 1, 5, mean);
    double range = calculateRange(tsArray, 1, 2);

    printf("%lf", std);

}