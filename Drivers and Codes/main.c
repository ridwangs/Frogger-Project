// Written by Ridwan Sadiq and Justin Bender

#include "LPC17xx.H"
#include "GLCD.h"
#include <stdlib.h>
#include <stdbool.h>
#include "uart.h"
#include "system_LPC17xx.h"
#include "GLCD_UTILS.h"
#include "Math.h"

extern volatile uint32_t UART0_Count;
extern volatile uint8_t UART0_Buffer[BUFSIZE];


void drawSquare(int cx, int cy, int width, int length, unsigned short color){
	int i, j;
	GLCD_SetTextColor(color);
	for(i = cx-width/2; i <= cx+width/2; i++){
		for(j = cy-length/2; j <= cy+length/2; j++){
			GLCD_PutPixel(i,j);
		}
	}
}

void dispLives(char x){
	unsigned char str[4] = "LV";
//	GLCD_SetBackColor(White);
	GLCD_SetTextColor(Blue);
	GLCD_DisplayString(8, 17, str);
	GLCD_DisplayChar(9, 18, x);
}

void dispScore(char x){
	unsigned char str[5] = "SC";
//	GLCD_SetBackColor(White);
	GLCD_SetTextColor(Blue);
	GLCD_DisplayString(3, 17, str);
	GLCD_DisplayChar(4, 18, x);
}

void initialize(char level[13][13], int frogLoc[]){
	int i,j;
	int px = 20;
	int py = 18;
	
	for(i = 0; i < 13; i++){
		for(j = 0; j < 13; j++){
			if (level[i][j] == 0) drawSquare(px*j+10, py*i+9, 20, 18 , Black);
			else if (level[i][j] == 1) drawSquare(px*j+10, py*i+9, 15, 10, Cyan);
			else if (level[i][j] == 2) drawSquare(px*j+10, py*i+9, 20, 10, Red);
			else if (level[i][j] == 3) drawSquare(px*j+10, py*i+9, 12, 10, Maroon);
			else if (level[i][j] == 4) drawSquare(px*j+10, py*i+9, 20, 18, Blue);
			else if (level[i][j] == 5) drawSquare(px*j+10, py*i+9, 20, 10, Brown);
			else if (level[i][j] == 6) drawSquare(px*j+10, py*i+9, 20, 18, Green);
			else if (level[i][j] == 7){
				if (i == 12) {
					drawSquare(px*j+10, py*i+13, 20, 18, Purple);
				}
				else{
					drawSquare(px*j+10, py*i+9, 20, 18, Purple);
				}
			}
			else{}
		}
	}
	
	drawSquare(frogLoc[1]*20+10, frogLoc[0]*18+9, 10, 10, White);
//	dispLives('0');
}
void drawFrog(int x, int y, int width, int length, unsigned short color){
	drawSquare(x*20+10, y*18+9, width, length, color);      // Frog Base
  
  drawSquare(x*20+7, y*18+6, width-7, length-7, White);   // Frog Eye 1
  drawSquare(x*20+13, y*18+6, width-7, length-7, White);  // Frog Eye 2

	drawSquare(x*20+3, y*18+9, width-3, length-8, Blue);  	// Frog Arm 1
	drawSquare(x*20+17, y*18+9, width-3, length-8, Blue);   // Frog Arm 2

	drawSquare(x*20+7, y*18+16, width-8, length-3, Blue);   // Frog Leg 1
	drawSquare(x*20+13, y*18+16, width-8, length-3, Blue);  // Frog Leg 2
}

int main() {
	// 0 --> Road 			(Black)
	// 1 --> Car 				(Cyan)
	// 2 --> Truck			(Red)
	// 3 --> Turtle			(Dark Purple)
	// 4 --> Water			(Blue)
	// 5 --> Log				(Brown)
	// 6 --> Home				(Green)
	// 7 --> Safe Zone	(Purple)
	GLCD_Init();
	GLCD_Clear(Black);

	int mask_valForward = 0x08000000;
  int mask_valBack = 0x10000000;
  int mask_valRight = 0x20000000;
  int mask_valLeft = 0x04000000;

	
//	int mask_valForward = 0x100;
//	int mask_valBack = 0x200000;
//	int mask_valRight = 0x2000;
//	int mask_valLeft = 0x1000;
	int on_val = 0x00000000;
	
	int i,j,k,t;
	char live = '3';
	int home = 5;
	char score = '0';
	char char_from_pc;
	UARTInit(0, 57600);
	
	char level[13][13] = {
		{6, 4, 4, 6, 4, 4, 6, 4, 4, 6, 4, 4, 6}, // Home and water  (Green and Blue)
		{4, 5, 5, 5, 4, 5, 5, 5, 4, 4, 5, 5, 5}, // Log and Water   (Brown and Blue)
		{3, 3, 4, 4, 4, 3, 3, 4, 4, 3, 3, 4, 4}, // Turtle and Water(Maroon and Blue)
		{4, 5, 5, 5, 4, 5, 5, 5, 4, 4, 5, 5, 5}, // Log and Water   (Brown and Blue)
		{3, 3, 4, 4, 4, 3, 3, 4, 4, 3, 3, 4, 4}, // Turtle and Water(Maroon and Blue)
		{4, 5, 5, 5, 4, 5, 5, 5, 4, 4, 5, 5, 5}, // Log and Water   (Brown and Blue)
		{7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7}, // Safe Zone       (Purple)
		{0, 0, 1, 0, 0, 0, 1, 0, 0, 1, 0, 0, 0}, // Car and Road    (White and Black)
		{0, 2, 2, 0, 0, 2, 2, 0, 0, 2, 2, 0, 0}, // Truck and Road  (White and Black)
		{0, 1, 0, 0, 0, 1, 0, 0, 1, 0, 0, 0, 0}, // Car and Road    (White and Black)
		{0, 0, 2, 2, 0, 0, 0, 2, 2, 0, 0, 0, 0}, // Truck and Road  (White and Black)
		{0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0}, // Car and Road    (White and Black)
		{7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7} // Safe Zone        (Purple)
	};
	int frogBase[] = {12, 6};
	int frogLoc [] = {12, 6};
	
	drawSquare(290,119, 60, 239, LightGrey);
	dispLives(live);
	dispScore(score);

	while(1){
		if (UART0_Count != 0 ) {
			char_from_pc = UART0_Buffer[0];						
			UARTSend( 0, (uint8_t *)UART0_Buffer, UART0_Count);
			UART0_Count = 0;
		}
				
		// Moving the Frog
		if(((LPC_GPIO1->FIOPIN & mask_valForward) == on_val) || (char_from_pc == 'w') ){// Moving Up
			char_from_pc = '0';
			if (frogLoc[0] == 0){
				frogLoc[0] -= 0;
			}
			else {
				frogLoc[0] -= 1;
				score+=1;
				dispScore(score);
			}
		}
		
		if((LPC_GPIO1->FIOPIN & mask_valBack) == on_val || (char_from_pc == 's')){// Moving Down
			char_from_pc = '0';
			if (frogLoc[0] == 12){
				frogLoc[0] -= 0;
			}
			else {
				frogLoc[0] += 1;
			}
		}
		
		if((LPC_GPIO1->FIOPIN & mask_valRight) == on_val || (char_from_pc == 'd')){// Moving Right
			char_from_pc = '0';
			if (frogLoc[1] == 12){
				frogLoc[1] += 0;
			}
			else {
				frogLoc[1] += 1;
			}
		}  
		
		if((LPC_GPIO1->FIOPIN & mask_valLeft) == on_val || (char_from_pc == 'a')){// Moving Left
			char_from_pc = '0';
			if (frogLoc[1] == 0){
				frogLoc[1] -= 0;
			}
			else {
				frogLoc[1] -= 1;
			}
		}
				
		// Collision with Cars or Trucks
		if (level[frogLoc[0]][frogLoc[1]] == 1 || level[frogLoc[0]][frogLoc[1]] == 2){// || level[frogLoc[0]][frogLoc[1]] == 4){
			drawFrog(frogLoc[1], frogLoc[0], 10, 10, Red);
			for(k = 0; k <30000000; k++);
			live -= 1;
			frogLoc[0] = frogBase[0];
			frogLoc[1] = frogBase[1];
			if(live == '0'){
				score = '0';
				GLCD_Init();
				GLCD_Clear(Black);	
				unsigned char str[14] = "GAME OVER :((";
				GLCD_SetTextColor(Blue);
				GLCD_DisplayString(5, 3, str);
				break;
			}
			dispLives(live);
		}
		if(level[frogLoc[0]][frogLoc[1]] == 3){
			if(frogLoc[1]<=0){
				drawFrog(frogLoc[1], frogLoc[0], 10, 10, Red);
				for(k = 0; k <30000000; k++);
				live -= 1;
				frogLoc[0] = frogBase[0];
				frogLoc[1] = frogBase[1];
				if(live == '0'){
					score = '0';
					GLCD_Init();
					GLCD_Clear(Black);	
					unsigned char str[14] = "GAME OVER :((";
					GLCD_SetTextColor(Blue);
					GLCD_DisplayString(5, 3, str);
					break;
				}
					dispLives(live);
			}
			else
				frogLoc[1]-=1; 
		}
		if(level[frogLoc[0]][frogLoc[1]] == 5){
			if(frogLoc[1]>=12){
				drawFrog(frogLoc[1], frogLoc[0], 10, 10, Red);
				for(k = 0; k <30000000; k++);
				live -= 1;
				frogLoc[0] = frogBase[0];
				frogLoc[1] = frogBase[1];
				if(live == '0'){
					score = '0';
					GLCD_Init();
					GLCD_Clear(Black);	
					unsigned char str[14] = "GAME OVER :((";
					GLCD_SetTextColor(Blue);
					GLCD_DisplayString(5, 3, str);
					break;
				}
					dispLives(live);
			}
			else
					if (frogLoc[0] != 3){
						frogLoc[1]+=1;
					}
					else{
						frogLoc[1]+=2;
					}
			}

		// Getting Home
		if (level[frogLoc[0]][frogLoc[1]] == 6){
			level[frogLoc[0]][frogLoc[1]] = 4;
			home -= 1;
			frogLoc[0] = frogBase[0];
			frogLoc[1] = frogBase[1];
			score +=5;
			dispScore(score);
			if(home == 0){
				GLCD_Init();
				GLCD_Clear(Black);	
				unsigned char str[22] = "CONGRATS! YOU WON";
				GLCD_SetTextColor(Blue);
				GLCD_DisplayString(5, 1, str);
				break;
			}
		}
		
		// Moving the Logs and Cars
		for(i = 0; i < 13; i++){
			if (i == 1 || i == 5 || i == 7 || i == 9 || i == 11){
				t = level[i][12];
				for(j = 0; j <= 11; j++){
					k = level[i][j];
					level[i][j] = t;
					t = k;
				}
				level[i][j] = t;
			}
			else if (i == 3){
				t = level[i][12];
				for(j = 0; j <= 11; j+=2){
					k = level[i][j];
					level[i][j] = t;
					t = k;
				}
				level[i][j] = t;
			}
			else if (i == 2 || i == 4 || i == 8 || i == 10){
				t = level[i][0];
				for(j = 12; j >= 1; j--){
					k = level[i][j];
					level[i][j] = t;
					t = k;
				}
				level[i][j] = t;
			}
		}

		initialize(level, frogLoc);	
		drawFrog(frogLoc[1], frogLoc[0], 10, 10, Green);
		}
}
