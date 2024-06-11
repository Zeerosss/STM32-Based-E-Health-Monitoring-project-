#include <ssd1306_pages.h>
#include <ssd1306_plots.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>

#include "ssd1306.h"
#include "ssd1306_conf.h"
#include "ssd1306_fonts.h"

// Definition of constants
#define DEFAULT_MIN_VALUE 1750
#define DEFAULT_MAX_VALUE 2250
#define DEFAULT_MIN_VALUE2 1700
#define DEFAULT_MAX_VALUE2 3350

// Function to initialize the OLED plot
plot_OLED ssd1306_PlotInit(uint32_t *buffer, uint16_t size, uint32_t *buffer2) {
    // Allocate memory for the plot_OLED structure
    plot_OLED p = malloc(sizeof(plot_struct));
    if (p != NULL) {
        // Initialize the values in the structure

        // Buffers
        p->buffer = buffer;
        p->buffer2 = buffer2;
        p->buffer_size = size;

        // Y-coordinates and temporary Y-coordinates
        p->y1 = 0;
        p->y2 = 0;
        p->y_temp = 0;
        p->y_temp2 = 0;

        // Counters and page number
        p->count = 0;
        p->page = 1;

        // Min and max Y-values
        p->min_y = 0;
        p->max_y = 0;
        p->min_y2 = 0;
        p->max_y2 = 0;

        // Min and max X-values
        p->min_x = 0;
        p->max_x = 0;

        // Min and max values for plotting
        p->min_value = 0;
        p->max_value = 0;
        p->min_value2 = 0;
        p->max_value2 = 0;
    }
    // Draw the first page on the OLED
    ssd1306_DrawPage1();
    return p;
}

// Function to change the page
void ssd1306_SetPage(plot_OLED S, uint8_t page) {
	S->page = page;

	if(S->page < 1 || S->page > 3) { // Force out some change page bug that can happen every once in a while
		page = 1;
		S->page = page;
	}
	switch (page) {
	case 1:
		// Configure parameters for page 1
		S->count = 0;
		S->min_y = 0;
		S->max_y = 0;
		S->min_y2 = 0;
		S->max_y2 = 0;
		S->min_x = 0;
		S->max_x = 0;
		S->min_value2 = 0;
		S->max_value2 = 0;
		ssd1306_DrawPage1();
		break;

	case 2:
		// Configure parameters for page 2
		S->count = MIN_X_page2;
		S->min_y = MIN_Y_page2_graph1;
		S->max_y = MAX_Y_page2_graph1;
		S->min_y2 = MIN_Y_page2_graph2;
		S->max_y2 = MAX_Y_page2_graph2;
		S->min_x = MIN_X_page2;
		S->max_x = MAX_X_page2;
		S->min_value = DEFAULT_MIN_VALUE;
		S->max_value = DEFAULT_MAX_VALUE;
		S->min_value2 = DEFAULT_MIN_VALUE2;
		S->max_value2 = DEFAULT_MAX_VALUE2;
		ssd1306_DrawPage2();
		break;

	case 3:
		// Configure parameters for page 3
		S->count = MIN_X_page3;
		S->min_y = MIN_Y_page3_graph1;
		S->max_y = MAX_Y_page3_graph1;
		S->min_y2 = MIN_Y_page3_graph2;
		S->max_y2 = MAX_Y_page3_graph2;
		S->min_x = MIN_X_page3;
		S->max_x = MAX_X_page3;
		S->min_value = 50; //heart rate
		S->max_value = 220;
		S->min_value2 = 10; //hrv
		S->max_value2 = 200;
		S->buffer_size = 1;
		ssd1306_DrawPage3();
		break;

	default:
		// Handle error when page is not found
		ssd1306_PrintMessage("Error finding page!");
		break;
	}
}

// Function to get the maximum X value for a given page
uint8_t ssd1306_GetMax_x(uint8_t page) {
	switch (page) {
	case 1:
		return MAX_X_page1;
	case 2:
		return MAX_X_page2;
	case 3:
		return MAX_X_page3;
	default:
		return 0;
	}
}

// Function to get the minimum X value for a given page
uint8_t ssd1306_GetMin_x(uint8_t page) {
	switch (page) {
	case 1:
		return MIN_X_page1;
	case 2:
		return MIN_X_page2;
	case 3:
		return MIN_X_page3;
	default:
		return 0;
	}
}

// Function to print an error message on the OLED
void ssd1306_PrintMessage(char *message) {
	ssd1306_Fill(Black);
	ssd1306_SetCursor(20, 20);
	ssd1306_WriteString(message, Font_7x10, White);
	ssd1306_UpdateScreen();
}

void ssd1306_PrintMessage2(char *message) {
	ssd1306_FillRectangle(20, 9, 110, 60, Black);
	ssd1306_SetCursor(35, 32);
	ssd1306_WriteString(message, Font_7x10, White);
	ssd1306_UpdateScreen();
}
// Function to print specific values on page 1
void ssd1306_PrintValuesPage1(uint16_t hr, float hrv) {
	char msg[30];

	ssd1306_FillRectangle(20, 9, 110, 60, Black);
	ssd1306_UpdateScreen();

	if (hr > 110) {
		ssd1306_SetCursor(23, 21);
		ssd1306_WriteString("HR high!", Font_7x10, White);
	} else if (hr < 49) {
		ssd1306_SetCursor(23, 21);
		ssd1306_WriteString("HR low!", Font_7x10, White);
	}

	if (hrv > 80) {
		ssd1306_SetCursor(23, 50);
		ssd1306_WriteString("HRV high!", Font_7x10, White);
	} else if (hrv < 10) {
		ssd1306_SetCursor(23, 50);
		ssd1306_WriteString("HRV low!", Font_7x10, White);
	}

	ssd1306_Line(23, 33, 30, 33, White);
	ssd1306_DrawPixel(33, 33, White);
	ssd1306_Line(36, 33, 90, 33, White);

	ssd1306_SetCursor(23, 9);

	sprintf(msg, "HR: %hubpm", hr);
	ssd1306_WriteString(msg, Font_7x10, White);
	ssd1306_UpdateScreen();

	if (hrv > 0) {
		ssd1306_SetCursor(23, 39);
		sprintf(msg, "HRV: %.1fms", hrv);
		ssd1306_WriteString(msg, Font_7x10, White);

		ssd1306_UpdateScreen();
	}
}

// Function to print specific values on page 3
void ssd1306_PrintValuesPage3(float dias, float synt, float pat) {
	char msg[40];

	// Print 'synt' value
	ssd1306_SetCursor(5, 40);
	sprintf(msg, "SYS:%.1fms", synt);
	ssd1306_WriteString(msg, Font_6x8, White);
	ssd1306_UpdateScreen();

	// Print 'pat' value
	ssd1306_SetCursor(66, 40);
	sprintf(msg, "PAT:%.1fms", pat);
	ssd1306_WriteString(msg, Font_6x8, White);
	ssd1306_UpdateScreen();

	// Print 'dias' value
	ssd1306_SetCursor(5, 50);
	sprintf(msg, "DIA:%.1fms", dias);
	ssd1306_WriteString(msg, Font_6x8, White);

	ssd1306_UpdateScreen();
}

// Function to reset the plot based on the current page
void ssd1306_ResetPlot(plot_OLED S) {
	switch (S->page) {

	case 2:
		ssd1306_FillRectangle(S->min_x, S->min_y - 1, S->max_x, S->max_y,
				Black);
		ssd1306_FillRectangle(S->min_x, S->min_y2 - 1, S->max_x, S->max_y2,
				Black);
		break;
	case 3:
		ssd1306_FillRectangle(S->min_x + 1, S->min_y - 1, S->max_x, S->max_y,
				Black);
		ssd1306_FillRectangle(S->min_x, S->min_y2 - 1, S->max_x, S->max_y2,
				Black);
		break;
	default:
		ssd1306_PrintMessage("Error finding page!");
		break;
	}
}

// Function to plot values on the OLED screen
void ssd1306_Plot(plot_OLED S) {
	int current_page = S->page;
	uint32_t A[250];
	uint32_t B[250];
	// Copy buffer data into local arrays
	memcpy(A, S->buffer, S->buffer_size * sizeof(uint32_t));
	memcpy(B, S->buffer2, S->buffer_size * sizeof(uint32_t));

	for (uint16_t i = 0; i < S->buffer_size; i++) {
		// Reset the screen if count exceeds the maximum X value
		if (S->count >= ssd1306_GetMax_x(S->page)) {
			S->count = ssd1306_GetMin_x(S->page);
			ssd1306_ResetPlot(S);
		}

		// Calculate Y coordinates for the plot based on the current page
		switch (S->page) {
		case 2:
			S->y1 = S->max_y
					- (((A[i] - S->min_value) * (S->max_y - S->min_y))
							/ (S->max_value - S->min_value));
			S->y2 = S->max_y2
					- (((B[i] - S->min_value2) * (S->max_y2 - S->min_y2))
							/ (S->max_value2 - S->min_value2));
			break;
		case 3:
			S->y1 = S->max_y
					- (((A[i] - S->min_value) * (S->max_y - S->min_y))
							/ (S->max_value - S->min_value));
			S->y2 = S->max_y2
					- (((B[i] - S->min_value2) * (S->max_y2 - S->min_y2))
							/ (S->max_value2 - S->min_value2));
			break;
		default:
			ssd1306_PrintMessage("Error finding page!");
			break;
		}
		if (S->y1 >= S->min_y && S->y1 <= S->max_y) {
			if (S->page == 2) {
				ssd1306_Line(S->count, S->y_temp, ++S->count, S->y1, White);
				S->y_temp = S->y1;
				if (S->y2 >= S->min_y2 && S->y2 <= S->max_y2) {
					ssd1306_Line(S->count - 1, S->y_temp2, S->count, S->y2,
							White);
					S->y_temp2 = S->y2;
				}

			} else {

				ssd1306_DrawPixel(++S->count, S->y1, White);
				if (S->y2 >= S->min_y2 && S->y2 <= S->max_y2) { ssd1306_DrawPixel(S->count, S->y2, White); }

			}
		}

		if (S->page != current_page)
			break;

	}
	ssd1306_UpdateScreen();
}
