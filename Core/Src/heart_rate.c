#include <heart_rate.h>
#include "stdio.h"
//use a control range >= of the moving average filter
//remember the size of rate_posiotion is equal to the size of R_intervals +1(beats+1)
float calculate_peak_intervals(uint32_t input[], float R_intervals[],
		uint16_t rate_position[], int size, int *beats, int sampling_time,
		int control_range) {
	int counter = 0;
	float sum = 0;
	int i, j;
	bool max_cond = true;
	for (i = control_range - 1; i < size - control_range; i++) {

		//Determination of the maximum through the examination of its neighborhood

		for (j = 1; j < control_range && max_cond; j++) {

			max_cond = max_cond
					&& (((input[i] >= input[i - j])
							&& (input[i] >= input[i + j])) && (input[i] != 0));

		}

		if (max_cond) {
			rate_position[counter] = i;
			counter++;
			i += control_range;

		}
		max_cond = true;
	}

	for (i = 1; i < counter; i++) {
		R_intervals[i - 1] = (rate_position[i] - rate_position[i - 1])
				* sampling_time;
		sum += R_intervals[i - 1];
	}
	if (counter <= 1)
		return 0;

	*beats = counter - 1;
	return sum / (counter - 1);
}

uint16_t find_minimum_position(uint32_t array[], int size) {
	uint16_t minimum = array[0];
	uint16_t minimum_position = 0;

	for (int i = 1; i < size; i++) {
		if (array[i] < minimum)
			minimum_position = i;
	}

	return minimum_position;
}

uint16_t remove_wrong(float array[], uint16_t size, float output[],
		uint32_t min_value, uint32_t max_value) {

	int counter = 0;

	for (int i = 0; i < size; i++) {
		if (array[i] > min_value && array[i] < max_value) {
			output[counter] = array[i];
			counter++;
		}
	}

	return counter;
}
uint32_t remove_wrong_int(uint32_t array[], uint16_t size, uint32_t output[],
		uint32_t min_value, uint32_t max_value) {
	int counter = 0;

	for (int i = 0; i < size; i++) {
		if (array[i] > min_value && array[i] < max_value) {
			output[counter] = array[i];
			counter++;
		}
	}

	return counter;
}
