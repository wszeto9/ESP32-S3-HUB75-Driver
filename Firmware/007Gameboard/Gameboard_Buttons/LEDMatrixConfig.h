#ifndef LEDMatrixConfig_h
#define LEDMatrixConfig_h

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
#define E_PIN 14 // required for 1/32 scan panels, like 64x64px. Any available pin would do, i.e. IO32
#define LAT_PIN 21
#define OE_PIN 13
#define CLK_PIN 47

#define PANEL_RES_X 64      // Number of pixels wide of each INDIVIDUAL panel module. 
#define PANEL_RES_Y 32     // Number of pixels tall of each INDIVIDUAL panel module.
#define PANEL_CHAIN 1      // Total number of panels chained one to another

#define BRIGHTNESS 255
#define displayRotation 3

#endif