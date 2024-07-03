#include <stdio.h>
#include <stdlib.h>
#include <math.h>

double calculateSubarrayMean(TimeSeries* ts, int startIndex, int endIndex) {
    // Validate indices
    if (startIndex < 0 || endIndex >= ts->size || startIndex > endIndex) {
        fprintf(stderr, "Invalid indices\n");
        return -1; // Error indicator
    }

    double sum = 0.0;
    int count = 0;

    // Sum the elements in the subarray
    for (int i = startIndex; i <= endIndex; i++) {
        sum += ts->data[i];
        count++;
    }

    // Calculate and return the mean
    return sum / count;
}