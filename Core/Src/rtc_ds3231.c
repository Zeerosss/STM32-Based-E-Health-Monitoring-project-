#include "rtc_ds3231.h"
#include "main.h"

#define ADDRESS_SIZE        1
#define DATA_TRANSFER_SIZE  7 //1 Second, 2 Minutes, 3 Hours, 4 Day, 5 Date, 6 Month, 7 Year

// Maximum number of tries
#define MAX_TRIES 3

// DEVICE ADDRESS (From the user guide)
#define DS3231_ADDRESS  0xD0 // It is 1101000<<1

#define DS3231_SECONDS  0x00
#define DS3231_MINUTES  0x01
#define DS3231_HOURS    0x02
#define DS3231_DAY      0x03
#define DS3231_DATE     0x04
#define DS3231_MONTH    0x05
#define DS3231_YEAR     0x06
#define DS3231_CONTROL  0x07

I2C_HandleTypeDef i2c_rtc;

// Function to convert BCD to decimal
uint8_t bcd_to_dec(uint8_t value) {
	uint8_t dec_value = ((value / 16 * 10) + (value % 16));
	return dec_value;
}

// Function to convert decimal to BCD
uint8_t dec_to_bcd(uint8_t value) {
	uint8_t bcd_value = ((value / 10 * 16) + (value % 10));
	return bcd_value;
}

// Function to initialize the DS3231 RTC
int8_t ds3231rtc_init(I2C_HandleTypeDef *hi2c) {
	i2c_rtc = *hi2c;
	HAL_StatusTypeDef ret;
	ret = HAL_I2C_IsDeviceReady(&i2c_rtc, DS3231_ADDRESS, MAX_TRIES,
			HAL_MAX_DELAY);
	if (ret != HAL_OK) {
		return -1;
	}
	return 0;
}

// Function to get date and time from the DS3231 RTC
int8_t ds3231rtc_get_date_time(date_time_type *date_time) {
	uint8_t buffer[DATA_TRANSFER_SIZE];
	HAL_StatusTypeDef ret;
	ret = HAL_I2C_Mem_Read(&i2c_rtc, DS3231_ADDRESS, DS3231_SECONDS,
			ADDRESS_SIZE, buffer, DATA_TRANSFER_SIZE, HAL_MAX_DELAY);
	if (ret != HAL_OK) {
		return -1;
	}
	// Convert BCD values to decimal and store in date_time struct
	date_time->seconds = bcd_to_dec(buffer[0]);
	date_time->minutes = bcd_to_dec(buffer[1]);
	date_time->hours = bcd_to_dec(buffer[2]);
	date_time->day = bcd_to_dec(buffer[3]);
	date_time->date = bcd_to_dec(buffer[4]);
	date_time->month = bcd_to_dec(buffer[5]);
	date_time->year = bcd_to_dec(buffer[6]);
	return 0;
}

// Function to set date and time to the DS3231 RTC
int8_t ds3231rtc_set_date_time(const date_time_type *date_time) {
	uint8_t buffer[DATA_TRANSFER_SIZE + ADDRESS_SIZE];
	HAL_StatusTypeDef ret;

	// Prepare data buffer with register addresses and data in BCD format
	buffer[0] = DS3231_SECONDS;
	buffer[1] = dec_to_bcd(date_time->seconds);
	buffer[2] = dec_to_bcd(date_time->minutes);
	buffer[3] = dec_to_bcd(date_time->hours);
	buffer[4] = dec_to_bcd(date_time->day);
	buffer[5] = dec_to_bcd(date_time->date);
	buffer[6] = dec_to_bcd(date_time->month);
	buffer[7] = dec_to_bcd(date_time->year);

	// Write data to the RTC
	ret = HAL_I2C_Mem_Write(&i2c_rtc, DS3231_ADDRESS, DS3231_SECONDS,
			ADDRESS_SIZE, buffer + 1, DATA_TRANSFER_SIZE, HAL_MAX_DELAY);
	if (ret != HAL_OK) {
		return -1;
	}
	return 0;
}
