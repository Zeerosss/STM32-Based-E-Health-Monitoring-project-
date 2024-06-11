#ifndef SIMPLE_FILTERS_H
#define SIMPLE_FILTERS_H

#include "stdint.h"

void moving_average_filter(uint32_t [], uint32_t [], uint16_t, uint8_t);
void low_pass_filter(uint32_t [], uint32_t [], uint16_t, float);
void high_pass_filter_flat(uint32_t [], uint16_t);
#endif /* SIMPLE_FILTERS_H */

