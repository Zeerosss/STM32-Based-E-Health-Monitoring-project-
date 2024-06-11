#ifndef STATISTICS_H
#define STATISTICS_H

#include <stdint.h>

float simple_mean(float values[], uint16_t size);
float int_simple_mean(uint32_t values[], uint16_t size);
float standard_deviation(float values[], uint16_t size);

#endif /* STATISTICS_H */
