/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file           : main.c
 * @brief          : Main program body
 ******************************************************************************
 * @attention
 *
 * Copyright (c) 2024 STMicroelectronics.
 * All rights reserved.
 *
 * This software is licensed under terms that can be found in the LICENSE file
 * in the root directory of this software component.
 * If no LICENSE file comes with this software, it is provided AS-IS.
 *
 ******************************************************************************
 */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "adc.h"
#include "dma.h"
#include "i2c.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "string.h"
#include "stdio.h"
#include "stdbool.h"

#include "ssd1306.h"
#include "ssd1306_plots.h"
#include "ssd1306_pages.h"
#include "rtc_ds3231.h"

#include "heart_rate.h"
#include "simple_filters.h"
#include "statistics.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */
#define SIZE 7650        // Size of the main buffer
#define MIN_SIZE 10	     // Size of the mini buffers
#define PERIOD 25	     // Moving average filter window
#define CONTROL_RANGE 75 // Range of control for the R-Interval function
#define ALPHA 0.1 		 // Smoothing coefficient -> can be used if the low pass filter is needed
#define SAMPLING_TIME 4  // 4 ms sampling time for the ADC modules
#define BEATS 60         // Constant for the BPMs (80 for a little bit of margin)
#define OLED_SIZE 5      // Size of the plot buffer for the second OLED mode -> 5 is the number of value that the OLED have at every plotting step
#define RANGE_MIN 1750   // Values used to check is the Pulse sensor is correctly applied to the patient (1750 - 2250)
#define RANGE_MAX 2250   //
#define RANGE_MIN2 1500  // Values used to check is the ECG sensor is correctly applied to the patient (1500 - 3500)
#define RANGE_MAX2 3500  //
#define MIN_DIASIS 50    // Min & Max Control values for the systolic and diastolic functions
#define MAX_DIASIS 1000  //
#define MIN_PAT 5        // Min & Max Control values for the PAT parameter (5-1000)
#define MAX_PAT  1000	 //
#define NUM_PAGES 3		 // Number of OLED pages
/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
uint32_t buffer[SIZE];					 // Main PPG Data buffer
uint32_t buffer_adc5[SIZE / 2];			 // Main ECG Data buffer
uint32_t mini_buffer[MIN_SIZE]; 		 // ADC1 (PPG) buffer
uint32_t mini_buffer_adc5[MIN_SIZE]; 	 // ADC5 (ECG) buffer
uint16_t fullness = 0;					 // Fullness state of the Main buffers (PPG & ECG)
uint32_t *reference = buffer;		     // PPG Buffer Pointer (for referencing it)
uint32_t *reference_adc5 = buffer_adc5;  // ECG Buffer Pointer (for referencing it)
float interval[BEATS];					 // Array of Inter-Beat Intervals (IBI) -> peak to peak interval in the PPG signal
float interval_adc5[BEATS];				 // Array of R-R intervals -> peak to peak interval in the ECG signal
uint16_t rate_position[BEATS];			 // Array of peaks positions in the PPG signal
uint16_t rate_position_adc5[BEATS];		 // Array of peaks positions in the ECG signal
int beats; 								 // Integer value indicating the total number of detected beats by the PPG
int beats_adc5;							 // Integer value indicating the total number of detected beats by the ECG
uint32_t buffer_oled[OLED_SIZE];		 // OLED Buffer for the first plot of the second mode
uint32_t buffer_oled2[OLED_SIZE];		 // OLED Buffer for the second plot of the second mode
int oled_count = 0;                      // OLED Buffer counter
plot_OLED plot;							 // OLED struct instance
date_time_type datetime;				 // Date Time struct instance

bool not_warning = false;				 // Boolean for the warning page
bool button_state = true;				 // Boolean for the button state
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
bool check_values(uint32_t[], uint16_t, uint16_t, uint16_t);
void handle_button_press(uint8_t*);
void reset_parameters(void);
/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_DMA_Init();
  MX_ADC1_Init();
  MX_I2C3_Init();
  MX_TIM2_Init();
  MX_TIM6_Init();
  MX_USART2_UART_Init();
  MX_ADC5_Init();
  MX_TIM7_Init();
  MX_TIM1_Init();
  /* USER CODE BEGIN 2 */

	// Datetime struct init
	if ((ds3231rtc_init(&hi2c3)) != 0) {
		char msg[100];
		sprintf(msg, "\nError with init of ds3231\n\r");
		HAL_UART_Transmit(&huart2, (uint8_t*) msg, strlen(msg), HAL_MAX_DELAY);
	}

	// Date time struct assegnation -> not all the RTCs are initialized
	datetime.minutes = 00;
	datetime.hours = 15;
	datetime.seconds = 00;
	datetime.date = 14;
	datetime.month = 06;
	datetime.year = 24;

	// Set time
	if (ds3231rtc_set_date_time(&datetime) != 0) {
		char msg[100];
		sprintf(msg, "\nError with set of ds3231\n\r");
		HAL_UART_Transmit(&huart2, (uint8_t*) msg, strlen(msg), HAL_MAX_DELAY);
	}

	ssd1306_Init(); 		//  OLED & plot initializers
	ssd1306_UpdateScreen();
	plot = ssd1306_PlotInit(buffer_oled, OLED_SIZE, buffer_oled2);

	HAL_TIM_Base_Start(&htim2);
	HAL_TIM_Base_Start_IT(&htim6);

	HAL_ADC_Start_DMA(&hadc1, mini_buffer, MIN_SIZE);
	HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_1);

	ssd1306_PrintMessage2("Loading...");
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
	while (1) {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
	}
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure the main internal regulator output voltage
  */
  HAL_PWREx_ControlVoltageScaling(PWR_REGULATOR_VOLTAGE_SCALE1);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLM = RCC_PLLM_DIV1;
  RCC_OscInitStruct.PLL.PLLN = 12;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = RCC_PLLQ_DIV2;
  RCC_OscInitStruct.PLL.PLLR = RCC_PLLR_DIV2;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV2;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_1) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */
// Check if a value in the ADCs mini buffers is out of range -> Correct sensor placement
bool check_values(uint32_t array[], uint16_t size, uint16_t range_min,
		uint16_t range_max) {

	bool within_range = true;

	// Check if all ADC values are within the range
	for (int i = 0; i < size; i++) {
		if (array[i] < range_min || array[i] > range_max) {
			within_range = false;
			break;
		}
	}
	return within_range;
}
// resets references
void reset_parameters(void) {
	reference_adc5 = buffer_adc5;
	reference = buffer;
	fullness = 0;
}

void handle_button_press(uint8_t *current_page) {

	if (*current_page + 1 > NUM_PAGES)
		*current_page = 1;
	else
		*current_page += 1;
}

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim) {
	uint16_t new_size = fullness; // saves the ADC buffer fullness because it is a global variable!
	uint32_t buffer2_adc1[SIZE];
	uint32_t buffer2_adc5[SIZE];

	float heart_rate;
	char msg[1000];

	if (htim->Instance == TIM1) {
		if (HAL_GPIO_ReadPin(button_GPIO_Port, button_Pin) == GPIO_PIN_SET) {

			HAL_TIM_Base_Stop_IT(&htim1); // We want to wait for the button state to be high, so we need to stop TIM1 here

			reset_parameters();
			oled_count = 0;		 // OLED Buffer counter reset
			handle_button_press(&(plot->page));

			sprintf(msg, "Current page: %hu \n\r", plot->page);
			HAL_UART_Transmit(&huart2, (uint8_t*) msg, strlen(msg), HAL_MAX_DELAY);

			ssd1306_SetPage(plot, plot->page);

			button_state = true;

			__HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_1, 1);
			__HAL_TIM_SET_COUNTER(&htim6, 0);					//Resets the timers counters
			__HAL_TIM_SET_COUNTER(&htim7, 0);					//

			switch (plot->page) {
			case 1:
				HAL_ADC_Stop_DMA(&hadc5);
				HAL_TIM_Base_Start_IT(&htim6); // Page 1 Displays HR & HRV according to TIM6
				HAL_TIM_Base_Stop_IT(&htim7);
				ssd1306_PrintMessage2("Loading...");
				break;
			case 2:
				HAL_TIM_Base_Stop_IT(&htim6);  // Page 2 Displays ECG & PPG plots
				HAL_ADC_Start_DMA(&hadc5, mini_buffer_adc5, MIN_SIZE);
				break;
			case 3:
				HAL_ADC_Start_DMA(&hadc5, mini_buffer_adc5, MIN_SIZE);	//
				HAL_TIM_Base_Start_IT(&htim7);							// Page 1 Displays HR & HRV according to TIM7
				break;
			default:
				break;
			}
		}
	}

	if (htim->Instance == TIM6) {

		if (new_size != 0) { // if the buffer is not empty
			HAL_ADC_Stop_DMA(&hadc1);
			reset_parameters();

			//Filters
			moving_average_filter(buffer, buffer2_adc1, new_size, PERIOD);
			high_pass_filter_flat(buffer2_adc1, new_size);	// Removes the average component
			high_pass_filter_flat(buffer2_adc1, new_size);	// Gets rid of the lower peaks
			high_pass_filter_flat(buffer2_adc1, new_size);	// Increases overall accuracy -> can be commented if necessary!

			float interval2[BEATS];
			// Works with references, interval will contain all the IPI intervals in the PPG signal, while rate_position the peaks positions in the buffer
			calculate_peak_intervals(buffer2_adc1, interval, rate_position,
					new_size, &beats,
					SAMPLING_TIME, CONTROL_RANGE);
			uint16_t right_beats = remove_wrong(interval, beats, interval2, 300, 1200); // Additional control of the Inter-Beat Intervals values (IBI)

			if ((ds3231rtc_get_date_time(&datetime)) != 0) {
				printf("\nError with get time\n\r");
			}

			float heart_rate_variability = standard_deviation(interval2, right_beats); // Calculates HRV
			heart_rate = 60000 / simple_mean(interval2, right_beats); // Calculates HR

			sprintf(msg,
					"Data from PulseSensor, acquired every 30s:\n\rDate: %.2d/%.2d/%.2d Time: %.2d:%.2d:%.2d\n\rHeart rate: %hu bpm\n\rHeart rate variability: %.1fms\n\rNumber of samples: %hu\n\r",
					datetime.date, datetime.month, datetime.year,
					datetime.hours, datetime.minutes, datetime.seconds,
					(uint16_t) heart_rate, heart_rate_variability, new_size);
			HAL_UART_Transmit(&huart2, (uint8_t*) msg, strlen(msg),
			HAL_MAX_DELAY);

			// USART2 transmits for abnormal HR & HRV values
			if (heart_rate < 49) {
				HAL_UART_Transmit(&huart2,
						(uint8_t *) "Be careful: low heart rate!",
						strlen("Be careful: low heart rate!"),
						HAL_MAX_DELAY);
			} else if (heart_rate > 110) {
				HAL_UART_Transmit(&huart2,
						(uint8_t *) "Take a deep breath: high heart rate!",
						strlen("Take a deep breath: high heart rate!"),
						HAL_MAX_DELAY);
			}

			if (heart_rate_variability < 10) {
				HAL_UART_Transmit(&huart2,
						(uint8_t *) "Be careful: low heart rate variability!",
						strlen("Be careful: low heart rate variability!"),
						HAL_MAX_DELAY);
			} else if (heart_rate_variability > 80) {
				HAL_UART_Transmit(&huart2,
						(uint8_t *) "Take a deep breath: high heart rate variability!",
						strlen("Take a deep breath: high heart rate variability!"),
						HAL_MAX_DELAY);
			}

			ssd1306_PrintValuesPage1((uint16_t) heart_rate, heart_rate_variability); // Display HR & HRV

			__HAL_TIM_SET_COUNTER(&htim6, 0);
			HAL_ADC_Start_DMA(&hadc1, mini_buffer, MIN_SIZE);
		}
	}

	if (htim->Instance == TIM7) {

		if (new_size > 20) { // if the buffer is not empty
			HAL_ADC_Stop_DMA(&hadc5);
			HAL_ADC_Stop_DMA(&hadc1);
			reset_parameters();

			moving_average_filter(buffer, buffer2_adc1, new_size, PERIOD);
			memcpy(buffer, buffer2_adc1, new_size * sizeof(uint32_t)); // SYS AND DIA are calculated after on a non high-filtered buffer -> we used memcpy
			high_pass_filter_flat(buffer2_adc1, new_size);			   // Here the previously filtered buffer is re-filtered and used for the R-R peaks calculation

			calculate_peak_intervals(buffer2_adc1, interval, rate_position, new_size, // For PPG IBI intervals
					&beats,
					SAMPLING_TIME, CONTROL_RANGE);

			moving_average_filter(buffer_adc5, buffer2_adc5, new_size, PERIOD / 5); // Now for the ECG part -> double filter
			high_pass_filter_flat(buffer2_adc5, new_size);
			calculate_peak_intervals(buffer2_adc5, interval_adc5, 	// Calculation of ECG R-R peaks intervals
					rate_position_adc5, new_size, &beats_adc5,
					SAMPLING_TIME, CONTROL_RANGE);

			float interval2[BEATS];
			uint16_t right_beats = remove_wrong(interval, beats, interval2, 300, 1000); // Additional control of the R-R Intervals values
			buffer_oled[0] = 60000 / simple_mean(interval2, right_beats); // For the HR plot in the third page
			buffer_oled2[0] = standard_deviation(interval2, right_beats); // For the HRV plot in the third page
			ssd1306_Plot(plot);

			int minimum_position;
			int closest_peak;
			float systolic[BEATS], diastolic[BEATS], pat[BEATS];

			if (beats > 1) { 		// We need at least 1 beat!

				for (int j = 1; j < beats + 1; j++) { // for all the PPG beats

					minimum_position = find_minimum_position(
							&buffer[rate_position[j - 1]],
							rate_position[j] - rate_position[j - 1]);

					systolic[j - 1] = (rate_position[j] - minimum_position
							- rate_position[j - 1]) * SAMPLING_TIME;
					diastolic[j - 1] = (minimum_position) * SAMPLING_TIME;   // SYS & DIA calculus

					if (beats_adc5 >= 2 && beats_adc5 <= beats + 2) { // Checks if the ECG and PPG beats are similar (the margin can be changed)

						for (int k = 0; k < beats_adc5; k++) { // and for all the ECG beats

							if (rate_position_adc5[k] < rate_position[j])  // Find the closest ECG-PPG peak variation
								closest_peak = k;

						}
						pat[j - 1] = (rate_position[closest_peak]
								- rate_position_adc5[j]) * SAMPLING_TIME; // PAT calculus
					}
				}

				uint16_t size_pat = 0;
				float systolic2[BEATS], diastolic2[BEATS], pat2[BEATS];
				uint16_t size_diastolic = remove_wrong(diastolic, beats,   // Additional control of the DIA values
						diastolic2, MIN_DIASIS, MAX_DIASIS);
				uint16_t size_systolic = remove_wrong(systolic, beats,    // Additional control of the SYS values
						systolic2, MIN_DIASIS, MAX_DIASIS);

				if (beats_adc5 >= 2 && beats_adc5 <= beats + 2)
					size_pat = remove_wrong(pat, beats - 1, pat2, MIN_PAT,  // Additional control of the PAT values
							MAX_PAT);

				if (size_diastolic == 0 || size_systolic == 0) {
					// UART DA SCRIVERE

				} else {

					ssd1306_PrintValuesPage3(
							simple_mean(diastolic2, size_diastolic),
							simple_mean(systolic2, size_systolic),
							simple_mean(pat2, size_pat));
					sprintf(msg, "Diastolic: %f Systolic: %f Pat: %f\n\r",
							simple_mean(diastolic2, size_diastolic),
							simple_mean(systolic2, size_systolic),
							simple_mean(pat2, size_pat));
					HAL_UART_Transmit(&huart2, (uint8_t*) msg, strlen(msg), HAL_MAX_DELAY);
				}
			}
		}
		__HAL_TIM_SET_COUNTER(&htim7, 0); 					 // Resets TIM7 counter
		HAL_ADC_Start_DMA(&hadc1, mini_buffer, MIN_SIZE);
		HAL_ADC_Start_DMA(&hadc5, mini_buffer_adc5, MIN_SIZE);
	}
}

void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef *hadc) {
	char msg[1000];

	if (hadc->Instance == ADC1) {

		if (fullness > SIZE)  // here we reset only if the array is exceeding in its size -> this could happen very rarely
			reset_parameters();

		fullness += MIN_SIZE; // Slight increase of the buffer fullness

		if (check_values(mini_buffer, MIN_SIZE, RANGE_MIN, RANGE_MAX)) { //Check if ADC values are within the correct range
			if (not_warning) {
				ssd1306_SetPage(plot, plot->page);

				not_warning = false;
				if (plot->page == 1)
					ssd1306_PrintMessage2("Loading..."); // Loading...
			}

			memcpy(reference, mini_buffer, MIN_SIZE * sizeof(uint32_t));
			reference += MIN_SIZE;

			if (plot->page == 2) { // If the current display page is page 2
				for(int i=0; i<MIN_SIZE; i++) { // USART2 transmit for raws PPG values
					sprintf(msg,"Value from PPG: %lu\n\r", mini_buffer[i]);
					HAL_UART_Transmit(&huart2, (uint8_t*) msg, strlen(msg), HAL_MAX_DELAY);
				}
			    if (oled_count >= OLED_SIZE) { // If the OLED counter has reached its maximum size, reset the OLED counter
			        oled_count = 0;
			        ssd1306_Plot(plot); // Plot page 2
			    }
			    buffer_oled[oled_count] = int_simple_mean(mini_buffer, MIN_SIZE); // Calculate the simple mean of the values in mini_buffer

			    buffer_oled2[oled_count] = int_simple_mean(mini_buffer_adc5, MIN_SIZE); // Calculate the simple mean of the values in mini_buffer_adc5
			    __HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_1, ((buffer_oled2[oled_count] - RANGE_MIN) * 398 ) / (RANGE_MAX - RANGE_MIN)); // Set the PULSE value for the LED signal based on the calculated mean value from buffer_oled2

			    oled_count++;
			}
			if (plot->page == 3) { // If the current display page is page 3
			    memcpy(reference_adc5, mini_buffer_adc5, MIN_SIZE * sizeof(uint32_t));
			    reference_adc5 += MIN_SIZE; // Move the reference_adc5 pointer forward by MIN_SIZE
			}
			} else { // If the ADC values are out of range

			    not_warning = true; // WARNING flag
			    ssd1306_DrawWarning();

			    __HAL_TIM_SET_COUNTER(&htim6, 0); // Reset the counters of TIM6 & TIM7
			    __HAL_TIM_SET_COUNTER(&htim7, 0); //
			    reset_parameters();
			    oled_count = 0;

			    sprintf(msg, "Error: ADC values out of range. Please reposition the sensor.\n\r");
			    HAL_UART_Transmit(&huart2, (uint8_t*) msg, strlen(msg), HAL_MAX_DELAY);
			}
	}

	if (hadc->Instance == ADC5) {

		if (!check_values(mini_buffer_adc5, MIN_SIZE, RANGE_MIN2,
					RANGE_MAX2)) { //Check if ADC values aren't within the correct range
			not_warning = true;
			ssd1306_DrawWarning();
			__HAL_TIM_SET_COUNTER(&htim6, 0); // Reset the counters of TIM6 & TIM7
			__HAL_TIM_SET_COUNTER(&htim7, 0); //

			reset_parameters();
			oled_count = 0;

			sprintf(msg,
					"Error: ADC values out of range. Please reposition the sensor.\n\r"); // Format an error message
			HAL_UART_Transmit(&huart2, (uint8_t*) msg, strlen(msg),
					HAL_MAX_DELAY); // Transmit the error message over UART
			} else {
				if (plot->page == 2) {
					for (int i = 0; i < MIN_SIZE; i++) {
						sprintf(msg, "Value from ECG: %lu\n\r",
								mini_buffer_adc5[i]);
						HAL_UART_Transmit(&huart2, (uint8_t*) msg, strlen(msg), // USART2 transmit for raws ECG values
								HAL_MAX_DELAY);

				}
			}
		}
	}
}

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin) {
	if (GPIO_Pin == button_Pin && button_state == true) { // Check if the interrupt was caused by the button press and the button is in the pressed state
		HAL_TIM_Base_Start_IT(&htim1); // Start timer 1 with interrupt enabled
		button_state = false; // Set button_state to false to indicate the button has been handled
	}
}

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
	/* User can add his own implementation to report the HAL error return state */
	__disable_irq();
	while (1) {
	}
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
