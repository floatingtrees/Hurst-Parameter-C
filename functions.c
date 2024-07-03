#include <stdio.h>
#include <stdlib.h>
#include <math.h>

typedef struct {
    double packet;
    double time;
} TimeSeries;

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


int main() {
     TimeSeries tsArray[5] = {
        {1.0, 1.1},
        {2.0, 2.2},
        {3.0, 3.3},
        {4.0, 4.4},
        {5.0, 5.5}
    };

    double x = calculateSubarrayMean(tsArray, 1, 4);
    printf("%lf", x);

}