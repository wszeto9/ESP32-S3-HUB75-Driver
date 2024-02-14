#define REV_A02

#include <ESP32-HUB75-MatrixPanel-I2S-DMA.h>
#include "LEDMatrixConfig.h"

MatrixPanel_I2S_DMA *dma_display = nullptr;

int counterHigh = 90;
int counterLow = 0;
int counterLowDisplay = 0;

int counterHighOld;
int counterLowOld;

unsigned long delayHigh = 0;
unsigned long delayLow = 0;
unsigned long delayHighOld = 0;
unsigned long delayLowOld = 0;

hw_timer_t * timer = NULL;


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
  timerISRInit();
}

void loop() {
  counterHigh++;
  delayHigh = 0;
  delay(2000);
  counterLow++;
  delayLow = 0;
  delay(1500);
}
