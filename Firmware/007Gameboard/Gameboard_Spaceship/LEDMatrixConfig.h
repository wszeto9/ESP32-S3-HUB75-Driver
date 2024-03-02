#ifndef LEDMatrixConfig_h
#define LEDMatrixConfig_h

#ifdef REV_A02

#define R1_PIN 42
#define G1_PIN 41
#define B1_PIN 40
#define R2_PIN 38
#define G2_PIN 39
#define B2_PIN 37
#define A_PIN 48
#define B_PIN 36
#define C_PIN 12
#define D_PIN 45
#define E_PIN 14
#define LAT_PIN 21
#define OE_PIN 13
#define CLK_PIN 47

#endif

#ifdef REV_B01

#define R1_PIN 41
#define G1_PIN 40
#define B1_PIN 39
#define R2_PIN 37
#define G2_PIN 38
#define B2_PIN 36
#define A_PIN 47
#define B_PIN 35
#define C_PIN 45
#define D_PIN 48
#define E_PIN 13
#define LAT_PIN 14
#define OE_PIN 12
#define CLK_PIN 21

#define NEOPIXEL_POWER 3
#define NEOPIXEL_DATA 46
#define ALS_PIN 1
#define TOP_BUTTON 10
#define BOTTOM_BUTTON 11
#define BUTTON_INPUT_1 2
#define BUTTON_INPUT_2 43
#define BUTTON_INPUT_3 44
#define BUTTON_INPUT_4 42

#endif

#define PANEL_RES_X 64      // Number of pixels wide of each INDIVIDUAL panel module. 
#define PANEL_RES_Y 32     // Number of pixels tall of each INDIVIDUAL panel module.
#define PANEL_CHAIN 1      // Total number of panels chained one to another

#define BRIGHTNESS 255
#define displayRotation 0

#endif