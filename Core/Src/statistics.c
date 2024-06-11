#include "statistics.h"
#include <math.h>

// Function to calculate the mean of an array of floats
float simple_mean(float values[], uint16_t size) {
    float sum = 0.0;

    // Calculate the sum of all values in the array
    for (int i = 0; i < size; i++) {
        sum += values[i];
    }
    // Return the mean by dividing the sum by the size of the array
    return sum / size;
}

// Function to calculate the mean of an array of unsigned integers
float int_simple_mean(uint32_t values[], uint16_t size) {
    float sum = 0.0;

    // Calculate the sum of all values in the array
    for (int i = 0; i < size; i++) {
        sum += values[i];
    }
    // Return the mean by dividing the sum by the size of the array
    return sum / size;
}
// Function to calculate the standard deviation of an array of floats
float standard_deviation(float values[], uint16_t size) {
    if (size == 0) {
        return 0.0;
    }

    float mean = simple_mean(values, size);
    float sum_of_squares = 0.0;

    // Calculate the sum of squares of differences between each value and the mean
    for (int i = 0; i < size; i++) {
        if (values[i] >= 150 &&values[i]<=1500) { // Consider values greater than or equal to 150
            float diff = values[i] - mean;
            sum_of_squares += diff * diff;
      }
    }

    // Return the square root of the mean of the sum of squares
    return sqrt(sum_of_squares / size);
}

