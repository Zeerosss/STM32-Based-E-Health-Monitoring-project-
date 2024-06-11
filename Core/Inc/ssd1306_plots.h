
#ifndef INC_SSD1306_PLOTS_H_
#define INC_SSD1306_PLOTS_H_

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include "i2c.h"

/*
     The graph area of the first page is:
     x=10 | x=118
     y=19 | y=55


     The graph areas of the second page are
     ECG
     x=34 | x=120
     y=13 | y=33

     PPG
     x=34 | x=120
     y=35 | y=54


     The graph areas of the third page are
     HR
     x=37 | x=118
     y=4  | y=15

     HRV
     x=37 | x=122
     y=21 | y=33
*/

//---------PAGE 1
#define MIN_X_page1 10
#define MAX_X_page1 118

#define MIN_Y_page1 19
#define MAX_Y_page1 55
//--------------


//--------PAGE 2
#define MIN_X_page2 34
#define MAX_X_page2 120

//---ECG
#define MIN_Y_page2_graph1 35
#define MAX_Y_page2_graph1 54

//---PPG
#define MIN_Y_page2_graph2 13
#define MAX_Y_page2_graph2 33
//--------------


//--------PAGE 3
#define MIN_X_page3 37
#define MAX_X_page3 118

//---HR
#define MIN_Y_page3_graph1 4
#define MAX_Y_page3_graph1 15

//---HRV
#define MIN_Y_page3_graph2 21
#define MAX_Y_page3_graph2 33
//--------------


typedef struct plot_struct{
	uint32_t *buffer;
	uint32_t *buffer2;
	uint8_t min_y;
	uint8_t max_y;
	uint8_t min_y2;
	uint8_t max_y2;
	uint8_t min_x;
	uint8_t max_x;
	uint8_t y1;
	uint8_t y2;
	uint8_t y_temp;
	uint8_t y_temp2;
	uint16_t min_value;
	uint16_t max_value;
	uint16_t min_value2;
	uint16_t max_value2;
	uint8_t count;
	uint8_t page;
	uint16_t buffer_size;
}plot_struct;

typedef struct plot_struct *plot_OLED;

plot_OLED ssd1306_PlotInit(uint32_t *buffer, uint16_t size,uint32_t*);

void ssd1306_SetPage(plot_OLED S, uint8_t page);

//void ssd1306_ClearBuffer(uint8_t *buffer);  //clear the OLED buffer in order to re-print something else

uint8_t ssd1306_GetMax_x(uint8_t page); //get the right X boundary where the graph has to end of the current page
uint8_t ssd1306_GetMin_x(uint8_t page); //get the left X boundary from where the graph has to start of the current page

void ssd1306_ResetPlot(plot_OLED S);
void ssd1306_Plot( plot_OLED S );
void ssd1306_PrintValuesPage1(uint16_t hr, float hrv);  //print "bpm" and "HRV" text + respective values on the first screen
void ssd1306_PrintValuesPage3(float dias, float synt, float pat); //print "SYN", "DIS" and "PAT" text + respective values on the third screen
void ssd1306_PrintMessage(char* message); //print on the OLED a message given as input
void ssd1306_PrintMessage2(char *message);
#endif /* INC_SSD1306_PLOTS_H_ */
