#include <ESP32-HUB75-MatrixPanel-I2S-DMA.h>
#include "LEDMatrixConfig.h"

MatrixPanel_I2S_DMA *dma_display = nullptr;

int counterHigh = 90;
int counterLow = 0;

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

void displayCounters(){

}

void setup() {
  initDisplay();
}

void loop() {
  for(int i = 0; i < 33; i ++){  
    dma_display->clearScreen();
    dma_display->setTextSize(2);
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
    if(counterLow >= 100){
      int hundreds = (counterLow - (counterLow % 100))/100;
      int tens = ((counterLow - hundreds * 100) - (counterHigh % 10))/10;
      int ones = counterLow % 10;
      dma_display->setCursor(0, 37);
      dma_display->print(hundreds);
      dma_display->setCursor(11, 37);
      dma_display->print(tens);
      dma_display->setCursor(22, 37);
      dma_display->print(ones);
    }
    else if(counterLow >= 10){
      dma_display->setCursor(5, 37);
      dma_display->println(counterLow);
    }
    else{
      dma_display->setCursor(11, 37);
      dma_display->println(counterLow);
    }
      dma_display->setCursor(21, 52);
      dma_display->setTextSize(1);
      dma_display->print(".5");
    dma_display->setTextSize(2);
    if(i == 32){
      dma_display->writeFillRect(0, 0, i, 5, dma_display->color444(0,15,0));
      dma_display->writeFillRect(0, 59, i, 5, dma_display->color444(0,8,0));
      delay(1000);
      counterHigh++;
      counterLow++;
    }
    else{
    dma_display->writeFillRect(0, 0, i, 5, 0xFF);
    dma_display->writeFillRect(0, 59, i, 5, 0xFF);
    }
    delayMicroseconds(500 * 1000 / 32);
  }
}
