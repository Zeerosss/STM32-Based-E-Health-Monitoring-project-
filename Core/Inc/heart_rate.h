#ifndef HEART_RATE_H
#define HEART_RATE_H

#include "stdint.h"
#include "stdbool.h"

float calculate_peak_intervals(uint32_t [], float[],uint16_t[], int, int *, int, int);
uint16_t remove_wrong(float array[],uint16_t size, float output[],uint32_t min_value, uint32_t max_value);
uint32_t remove_wrong_int(uint32_t array[],uint16_t size, uint32_t output[],uint32_t min_value, uint32_t max_value);
uint16_t find_minimum_position(uint32_t array[],int size);
#endif /* HEARTRATE_H */

