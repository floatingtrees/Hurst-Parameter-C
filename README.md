# Hurst Parameter Calculations in C

Compile FeatureExtract2.c with 
```
gcc-14 -fopenmp FeatureExtract2.c -o FeatureExtract2
```
on Mac or
```
gcc -fopenmp FeatureExtract2.c -o FeatureExtract2 -lm
```
on Linux. Then, run with
```
./FeatureExtract2
```
FeatureExtract2 takes in a csv file containing time (with each time on a newline), and another csv containing num_packets. They should have the same length. It calculates the hurst parameter for every 10 milliseconds, so packets in the same 10 millisecond window are added together. You should switch commas in the CSV to newlines so the parser properly reads the numbers as doubles. To increase speed, you can also replace all doubles with floats. 
