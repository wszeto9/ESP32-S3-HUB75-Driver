//Display Refresh is on a ~ 60Hz timer interrupt. It grabs data from global variables and redraws the display if needed

//Buttons are on a pin change interrupt. They edit the values of the last time a button is pressed. 

#define REV_B01

#include <ESP32-HUB75-MatrixPanel-I2S-DMA.h>
#include "LEDMatrixConfig.h"
#include "ImageAssets.h"

MatrixPanel_I2S_DMA *dma_display = nullptr;

int counterHigh = 0;
int counterLow = 0;
int counterLowDisplay = 0;

int counterHighOld;
int counterLowOld;

unsigned long delayHigh = 0;
unsigned long delayLow = 0;
unsigned long delayHighOld = 0;
unsigned long delayLowOld = 0;

hw_timer_t * timer = NULL;

unsigned long lastButtonPressHighTime;
unsigned long lastButtonPressLowTime;

uint8_t lastButtonHigh;
uint8_t lastButtonLow;

void handleButtonInterrupts(uint8_t buttonNumber){
  if(buttonNumber > 2 && lastButtonHigh != buttonNumber && (lastButtonPressHighTime + 500) < millis()){
    counterHigh ++;
    lastButtonPressHighTime = millis();
    delayHigh = 0;
    lastButtonHigh = buttonNumber;
  }
  else if(buttonNumber < 3 && lastButtonLow != buttonNumber && (lastButtonPressLowTime + 500) < millis()){
    counterLow ++;
    lastButtonPressLowTime = millis();
    delayLow = 0;
    lastButtonLow = buttonNumber;
  }
}

void IRAM_ATTR ButtonInterruptFunction1(){
  handleButtonInterrupts(1);
}
void IRAM_ATTR ButtonInterruptFunction2(){
  handleButtonInterrupts(2);
}
void IRAM_ATTR ButtonInterruptFunction3(){
  handleButtonInterrupts(3);
}
void IRAM_ATTR ButtonInterruptFunction4(){
  handleButtonInterrupts(4);
}

void drawXbm565(int x, int y, int width, int height, const char *xbm, uint16_t color = 0xffff) 
{
  if (width % 8 != 0) {
      width =  ((width / 8) + 1) * 8;
  }
    for (int i = 0; i < width * height / 8; i++ ) {
      unsigned char charColumn = pgm_read_byte(xbm + i);
      for (int j = 0; j < 8; j++) {
        int targetX = (i * 8 + j) % width + x;
        int targetY = (8 * i / (width)) + y;
        if (bitRead(charColumn, j)) {
          dma_display->drawPixel(targetX, targetY, color);
        }
      }
    }
}

void initDisplay(){
  HUB75_I2S_CFG::i2s_pins _pins={R1_PIN, G1_PIN, B1_PIN, R2_PIN, G2_PIN, B2_PIN, A_PIN, B_PIN, C_PIN, D_PIN, E_PIN, LAT_PIN, OE_PIN, CLK_PIN};
  HUB75_I2S_CFG mxconfig(PANEL_RES_X, PANEL_RES_Y, PANEL_CHAIN, _pins);
  mxconfig.clkphase = false;
  dma_display = new MatrixPanel_I2S_DMA(mxconfig);
  dma_display->setRotation(displayRotation);
  dma_display->setTextWrap(false);
  dma_display->begin();
  dma_display->setTextSize(2);  
  dma_display->setBrightness8(BRIGHTNESS); //0-255
}

void buttonSetup(){
  pinMode(BUTTON_INPUT_1, INPUT_PULLUP);
  pinMode(BUTTON_INPUT_2, INPUT_PULLUP);
  pinMode(BUTTON_INPUT_3, INPUT_PULLUP);
  pinMode(BUTTON_INPUT_4, INPUT_PULLUP);

  while(digitalRead(BUTTON_INPUT_1)|digitalRead(BUTTON_INPUT_2)|digitalRead(BUTTON_INPUT_3)|digitalRead(BUTTON_INPUT_4)){
    dma_display->setBrightness8(100); //0-255
    drawXbm565(0,0,32,64, SwitchDisconnectedRed, dma_display->color565(255,0,0));
    drawXbm565(0,0,32,64, SwitchDisconnectedGrey, dma_display->color565(128,128,128));
    drawXbm565(0,0,32,64, SwitchDisconnectedWhite, dma_display->color565(128,128,128));
    dma_display->setTextColor(dma_display->color444(255, 0, 0));
    dma_display->setTextSize(1);
    dma_display->setCursor(13,12);
    if(digitalRead(BUTTON_INPUT_1)){
      dma_display->println("1");
    }
    else if(digitalRead(BUTTON_INPUT_2)){
      dma_display->println("2");
    }
    else if(digitalRead(BUTTON_INPUT_3)){
      dma_display->println("3");
    }
    else if(digitalRead(BUTTON_INPUT_4)){
      dma_display->println("4");
    }
    delay(100);
  }
  dma_display->setBrightness8(BRIGHTNESS); //0-255
  attachInterrupt(digitalPinToInterrupt(BUTTON_INPUT_1), ButtonInterruptFunction1, CHANGE);
  attachInterrupt(digitalPinToInterrupt(BUTTON_INPUT_2), ButtonInterruptFunction2, CHANGE);
  attachInterrupt(digitalPinToInterrupt(BUTTON_INPUT_3), ButtonInterruptFunction3, CHANGE);
  attachInterrupt(digitalPinToInterrupt(BUTTON_INPUT_4), ButtonInterruptFunction4, CHANGE);
}




void IRAM_ATTR timerISR() {
  if(!((counterHigh == counterHighOld) && (counterLow == counterLowOld) && ((delayHigh == delayHighOld) || delayHigh > 32) && ((delayLow == delayLowOld) || delayLow > 32)))
  {
    dma_display->clearScreen();
    dma_display->setTextSize(2);
    updatePointsDisplay();
    updateDelayBarsDisplay();

    counterHighOld = counterHigh;
    counterLowOld = counterLow;
    delayHighOld = delayHigh;
    delayLowOld = delayLow;
  }
  delayHigh++;
  delayLow++;


}

void timerISRInit(){
  timer = timerBegin(0, 80, true); // Timer 0, divider 80
  timerAttachInterrupt(timer, &timerISR, true); // Attach ISR
  timerAlarmWrite(timer, 15625, true); // updates 32 times in a 500ms period
  timerAlarmEnable(timer); // Enable the timer
}

void updatePointsDisplay(){
  if(counterHigh >= 100){
    int hundreds = (counterHigh - (counterHigh % 100))/100;
    int tens = ((counterHigh - hundreds * 100) - (counterHigh % 10))/10;
    int ones = counterHigh % 10;
    dma_display->setCursor(0, 13);
    dma_display->print(hundreds);
    dma_display->setCursor(11, 13);
    dma_display->print(tens);
    dma_display->setCursor(22, 13);
    dma_display->print(ones);
    }
    else if(counterHigh >= 10){
      dma_display->setCursor(5, 13);
      dma_display->println(counterHigh);
    }
    else{
      dma_display->setCursor(11, 13);
      dma_display->println(counterHigh);
    }

    counterLowDisplay = counterLow / 2;
    if(counterLowDisplay >= 100){
      int hundreds = (counterLowDisplay - (counterLowDisplay % 100))/100;
      int tens = ((counterLowDisplay - hundreds * 100) - (counterHigh % 10))/10;
      int ones = counterLowDisplay % 10;
      dma_display->setCursor(0, 37);
      dma_display->print(hundreds);
      dma_display->setCursor(11, 37);
      dma_display->print(tens);
      dma_display->setCursor(22, 37);
      dma_display->print(ones);
    }
    else if(counterLowDisplay >= 10){
      dma_display->setCursor(5, 37);
      dma_display->println(counterLowDisplay);
    }
    else{
      dma_display->setCursor(11, 37);
      dma_display->println(counterLowDisplay);
    }
    if(counterLow % 2){
      dma_display->setCursor(21, 52);
      dma_display->setTextSize(1);
      dma_display->print(".5");
      dma_display->setTextSize(2);
    }
}

void updateDelayBarsDisplay(){
  if(delayHigh >= 32){
    dma_display->writeFillRect(0, 0, 32, 5, dma_display->color444(0,15,0));
  }
  else{
    dma_display->writeFillRect(0, 0, delayHigh, 5, 0xFF);
  }

  if(delayLow >= 32){
    dma_display->writeFillRect(0, 59, 32, 5, dma_display->color444(0,15,0));
  }
  else{
    dma_display->writeFillRect(0, 59, delayLow, 5, 0xFF);
  }
}

void setup() {
  Serial.begin(115200);
  initDisplay();
  buttonSetup();
  timerISRInit();
}

void loop() {
  Serial.print("Counter High: ");
  Serial.print(counterHigh);
  Serial.print(", Counter Low: ");
  Serial.println(counterLow);
  delay(100);
}
