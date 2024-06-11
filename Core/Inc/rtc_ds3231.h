#ifndef INC_RTC_DS3231_H_
#define INC_RTC_DS3231_H_
#include "main.h"

struct date_time
{
	uint8_t     seconds;
	uint8_t     minutes;
	uint8_t     hours;
	uint8_t     day;
	uint8_t     date;
	uint8_t     month;
	uint16_t    year;
};
typedef struct date_time date_time_type;



int8_t ds3231rtc_init(I2C_HandleTypeDef * hi2c);

int8_t ds3231rtc_get_date_time(date_time_type * date_time);

int8_t ds3231rtc_set_date_time(const date_time_type * date_time);


#endif /* INC_RTC_DS3231_H_ */
