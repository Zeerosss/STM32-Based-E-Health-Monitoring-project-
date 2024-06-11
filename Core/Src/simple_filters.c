#include "simple_filters.h"
#include "statistics.h"
#include "math.h"

// Function to apply a moving average filter to an array of integers
void moving_average_filter(uint32_t input[], uint32_t output[], uint16_t size,
		uint8_t period) {
	int i, j;
	float sum;

	// Apply the moving average filter to each element of the input array
	for (i = 0; i < size; i++) {
		sum = 0.0;
		uint8_t count = 0;
		// Calculate the sum of elements within the window defined by the period
		for (j = i - period; j <= i + period; j++) {
			if (j >= 0 && j < size) {
				sum += input[j];
				count++;
			}
		}
		// Calculate the mean of the elements and round to the nearest integer
		output[i] = (int) ((sum / count) + 0.5);
	}
}

// Function to apply a low-pass filter to an array of integers
void low_pass_filter(uint32_t input[], uint32_t output[], uint16_t size,
		float alpha) {
	if (alpha < 0.0 || alpha > 1.0) {
		// Alpha (smoothing coefficient)
		return;
	}

	output[0] = input[0];

	// Apply the low-pass filter to each element of the input array
	for (int i = 1; i < size; i++) {
		output[i] = (uint32_t) (alpha * input[i] + (1 - alpha) * output[i - 1]);
	}
}

// Function to apply a flat high-pass filter to an array of integers
void high_pass_filter_flat(uint32_t input[], uint16_t size) {
	// Calculate the mean of the input array
	uint32_t mean = (uint32_t) int_simple_mean(input, size);
	int i;
	int *array = input; /*This cast to int is useful to convert the number to signed
						if the subtraction returns a negative number*/
	// Apply the flat high-pass filter to each element of the input array
	for (i = 0; i < size; i++) {
		array[i] = input[i] - mean;
		if ((array[i]) < 0) {
			array[i] = 0;
		}
	}
}
