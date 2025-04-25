//Display Refresh is on a ~ 60Hz timer interrupt. It grabs data from global variables and redraws the display if needed

//Buttons are on a pin change interrupt. They edit the values of the last time a button is pressed. 

#define REV_B01

#include <ESP32-HUB75-MatrixPanel-I2S-DMA.h>
#include "LEDMatrixConfig.h"
#include "ImageAssets.h"
#include <Adafruit_MCP23X17.h>
Adafruit_MCP23X17 mcp;

MatrixPanel_I2S_DMA *dma_display = nullptr;

int counterHigh = 0;
int counterLow = 0;
int counterThree = 0; 
int counterLowDisplay = 0;
int points = -3; 
int oldPoints = -3; 

int counterHighOld;
int counterLowOld;
int counterThreeOld; 

unsigned long delayHigh = 0;
unsigned long delayLow = 0;
unsigned long delayThree = 0; 
unsigned long delayHighOld = 0;
unsigned long delayLowOld = 0;
unsigned long delayThreeOld = 0; 

hw_timer_t * timer = NULL;

unsigned long lastButtonPressHighTime;
unsigned long lastButtonPressLowTime;
unsigned long lastButtonPressThreeTime; 

uint8_t lastButtonHigh;
uint8_t lastButtonLow;
uint8_t lastButtonThree; 

#define BlueSide

//yellow
#ifdef YellowWSide
#define colorDark 0x72a0
#define colorLight 0xfde0
#endif

//green
#ifdef GreenSide
#define colorLight 0x25a0
#define colorDark 0x1340
#endif

#ifdef RedSide
#define colorLight 0xF800
#define colorDark 0x7000
#endif

#ifdef BlueSide
#define colorLight 0x001F
#define colorDark 0x0010
#endif

void handleButtonInterrupts(uint8_t buttonNumber){
  if(buttonNumber > 2){
    if(lastButtonHigh == buttonNumber){
      lastButtonPressHighTime = millis();
      delayHigh = 0;
    }
    if(lastButtonHigh != buttonNumber && (lastButtonPressHighTime + 500) < millis()){
      counterHigh ++;
      lastButtonHigh = buttonNumber;
      lastButtonPressHighTime = millis();
      delayHigh = 0;
    }
  }

  if(buttonNumber < 3){
    if(lastButtonLow == buttonNumber){
      lastButtonPressLowTime = millis();
      delayLow = 0;
    }
    if(lastButtonLow != buttonNumber && (lastButtonPressLowTime + 500) < millis()){
      counterLow ++;
      lastButtonLow = buttonNumber;
      lastButtonPressLowTime = millis();
      delayLow = 0;
    }
  }
}

void IRAM_ATTR ButtonInterruptFunction1(){
  if(mcp.digitalRead(BUTTON_INPUT_1)){
    if(lastButtonLow == 1){
        lastButtonPressLowTime = millis();
        delayLow = 0;
      }
    if(lastButtonLow != 1 && (abs(lastButtonPressLowTime - millis()) > 1000)){
      counterLow ++;
      lastButtonLow = 1;
      lastButtonPressLowTime = millis();
      delayLow = 0;
    } 
  }
  //handleButtonInterrupts(1);
}
void IRAM_ATTR ButtonInterruptFunction2(){
  if(mcp.digitalRead(BUTTON_INPUT_2)){
    if(lastButtonLow == 2){
        lastButtonPressLowTime = millis();
        delayLow = 0;
      }
    if(lastButtonLow != 2 && (abs(lastButtonPressLowTime - millis()) > 1000)){
      counterLow ++;
      lastButtonLow = 2;
      lastButtonPressLowTime = millis();
      delayLow = 0;
    }
  }
  //handleButtonInterrupts(2);
}
void IRAM_ATTR ButtonInterruptFunction3(){
  //Serial.println("Button 3"); 
 //Serial.println(mcp.digitalRead(BUTTON_INPUT_3)); 
  if(mcp.digitalRead(BUTTON_INPUT_3)){
    if(lastButtonHigh == 3){
      lastButtonPressHighTime = millis();
      delayHigh = 0;
    }
    if(lastButtonHigh != 3 && && (abs(lastButtonPressHighTime - millis()) > 1000)){
      counterHigh ++;
      lastButtonHigh = 3;
      lastButtonPressHighTime = millis();
      delayHigh = 0;
    }
  }
  //handleButtonInterrupts(3);
}
void IRAM_ATTR ButtonInterruptFunction4(){
  //Serial.println("Button 4"); 
  //Serial.println(mcp.digitalRead(BUTTON_INPUT_4)); 
  if(mcp.digitalRead(BUTTON_INPUT_4)){
    if(lastButtonHigh == 4){
      lastButtonPressHighTime = millis();
      delayHigh = 0;
    }
    if(lastButtonHigh != 4 && (abs(lastButtonPressHighTime - millis()) > 1000)){
      counterHigh ++;
      lastButtonHigh = 4;
      lastButtonPressHighTime = millis();
      delayHigh = 0;
    }
  }
  //handleButtonInterrupts(4);
}

void IRAM_ATTR ButtonInterruptFunction5(){
  //Serial.println("Button 4"); 
  //Serial.println(mcp.digitalRead(BUTTON_INPUT_4)); 
  if(mcp.digitalRead(BUTTON_INPUT_5)){
    if(lastButtonThree == 5){
      lastButtonPressThreeTime = millis();
      delayThree = 0;
    }
    if(lastButtonThree != 5 && (abs(lastButtonPressThreeTime - millis()) > 1000)){
      counterThree ++;
      lastButtonThree = 5;
      lastButtonPressThreeTime = millis();
      delayThree = 0;
    }
  }
  //handleButtonInterrupts(4);
}

void IRAM_ATTR ButtonInterruptFunction6(){
  //Serial.println("Button 4"); 
  //Serial.println(mcp.digitalRead(BUTTON_INPUT_4)); 
  if(mcp.digitalRead(BUTTON_INPUT_6)){
    if(lastButtonThree == 6){
      lastButtonPressThreeTime = millis();
      delayThree = 0;
    }
    if(lastButtonThree != 6 && (abs(lastButtonPressThreeTime - millis()) > 1000)){
      counterThree ++;
      lastButtonThree = 6;
      lastButtonPressThreeTime = millis();
      delayThree = 0;
    }
  }
  //handleButtonInterrupts(4);
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
  dma_display->setTextSize(6);  
  dma_display->setBrightness8(BRIGHTNESS); //0-255
}

void buttonSetup(){
  /*pinMode(BUTTON_INPUT_1, INPUT_PULLUP);
  pinMode(BUTTON_INPUT_2, INPUT_PULLUP);
  pinMode(BUTTON_INPUT_3, INPUT_PULLUP);
  pinMode(BUTTON_INPUT_4, INPUT_PULLUP);*/ 

  if (!mcp.begin_I2C()) {
  //if (!mcp.begin_SPI(CS_PIN)) {
    Serial.println("Error.");
    while (1);
  }

  // configure pin for input with pull up
  mcp.pinMode(BUTTON_INPUT_1, INPUT_PULLUP);
  mcp.pinMode(BUTTON_INPUT_2, INPUT_PULLUP);
  mcp.pinMode(BUTTON_INPUT_3, INPUT_PULLUP);
  mcp.pinMode(BUTTON_INPUT_4, INPUT_PULLUP);
  mcp.pinMode(BUTTON_INPUT_5, INPUT_PULLUP);
  mcp.pinMode(BUTTON_INPUT_6, INPUT_PULLUP);

  int b1 = mcp.digitalRead(BUTTON_INPUT_1); 
  int b2 = mcp.digitalRead(BUTTON_INPUT_2); 
  int b3 = mcp.digitalRead(BUTTON_INPUT_3); 
  int b4 = mcp.digitalRead(BUTTON_INPUT_4); 
  int b5 = mcp.digitalRead(BUTTON_INPUT_5); 
  int b6 = mcp.digitalRead(BUTTON_INPUT_6); 


  while(b1|b2|b3|b4|b5|b6){

    int even = 0; 
    int odd = 0; 
    int sum = 0; 

    if(b1){
      sum++; 
      odd = 1; 
    }
    if(b2){
      sum++;
      even = 1; 
    }
    if(b3){
      sum++; 
      odd = 1; 
    }
    if(b4){
      sum++;
      even = 1; 
    }
    if(b5){
      sum++; 
      odd = 1; 
    }
    if(b6){
      sum++;
      even = 1; 
    }
    
    Serial.println(sum);
    if(sum != 3){
      dma_display->setBrightness8(100); //0-255
      drawXbm565(16,0,32,32, SwitchDisconnectedRed, dma_display->color565(255,0,0));
      drawXbm565(16,0,32,32, SwitchDisconnectedGrey, dma_display->color565(128,128,128));
      drawXbm565(16,0,32,32, SwitchDisconnectedWhite, dma_display->color565(128,128,128));
      dma_display->setTextColor(dma_display->color444(255, 0, 0));
      dma_display->setTextSize(1);
      dma_display->setCursor(27,9);

      if(even){
        dma_display->println("0");
      }
      else if(odd){
        dma_display->println("1");
      }

    } else {
      break;
    }

    
    delay(100);
  }

  dma_display->setBrightness8(BRIGHTNESS); //0-255
  attachInterrupt(digitalPinToInterrupt(BUTTON_INPUT_1), ButtonInterruptFunction1, RISING);
  attachInterrupt(digitalPinToInterrupt(BUTTON_INPUT_2), ButtonInterruptFunction2, RISING);
  attachInterrupt(digitalPinToInterrupt(BUTTON_INPUT_3), ButtonInterruptFunction3, RISING);
  attachInterrupt(digitalPinToInterrupt(BUTTON_INPUT_4), ButtonInterruptFunction4, RISING);
  attachInterrupt(digitalPinToInterrupt(BUTTON_INPUT_5), ButtonInterruptFunction5, RISING);
  attachInterrupt(digitalPinToInterrupt(BUTTON_INPUT_6), ButtonInterruptFunction6, RISING);

}

void IRAM_ATTR timerISR() {
  if(!((counterHigh == counterHighOld) && (counterLow == counterLowOld) && ((delayHigh == delayHighOld) || delayHigh > 32) && ((delayLow == delayLowOld) || delayLow > 32)))
  {
    dma_display->clearScreen();
    dma_display->setTextSize(2);
    updatePointsDisplay();
    //updateDelayBarsDisplay();

    //horizontal bars 
    dma_display->writeFillRect(0, 0, 64, 5, colorLight);
    dma_display->writeFillRect(0, 27, 64, 5, colorLight);

    //vertical bars 
    dma_display->writeFillRect(0, 0, 5, 32, colorLight);
    dma_display->writeFillRect(59, 0, 5, 32, colorLight);

    counterHighOld = counterHigh;
    counterLowOld = counterLow;
    counterThreeOld = counterThree; 
    delayHighOld = delayHigh;
    delayLowOld = delayLow;
    delayThreeOld = delayThree; 
  }
  delayHigh++;
  delayLow++;
  delayThree++;


}

void timerISRInit(){
  timer = timerBegin(60000); // 60 kHz timer
  timerAttachInterrupt(timer, &timerISR); // Attach ISR
  timerAlarm(timer, 1000, true, 0); //calls interrupt every 60kHz/1000 = 60 Hz
}

void updatePointsDisplay(){
  //horizontal bars 
  dma_display->writeFillRect(0, 0, 64, 5, colorLight);
  dma_display->writeFillRect(0, 27, 64, 5, colorLight);

  //vertical bars 
  dma_display->writeFillRect(0, 0, 5, 32, colorLight);
  dma_display->writeFillRect(59, 0, 5, 32, colorLight);

  if(points >= 100){
    int hundreds = (points - (points % 100))/100;
    int tens = ((points - hundreds * 100) - (points % 10))/10;
    int ones = points % 10;
    // Serial.print("High Hundreds: ");
    // Serial.print(hundreds);
    // Serial.print(", tens: ");
    // Serial.print(tens);
    // Serial.print(", ones: ");
    // Serial.println(ones);
    dma_display->setTextColor(0xFFFF, 0x0);
    dma_display->setCursor(15, 9);
    dma_display->print(hundreds);
    dma_display->setCursor(27, 9);
    dma_display->print(tens);
    dma_display->setCursor(39, 9);
    dma_display->print(ones);
    }
    else if(points >= 10){
      dma_display->setCursor(21, 9);
      dma_display->println(points);
    }
    else{
      dma_display->setCursor(27, 9); // x originally 11
      dma_display->println(points);
    }
/*
    counterLowDisplay = counterLow / 2;
    if(counterLowDisplay >= 100){
      int hundreds = (counterLowDisplay - (counterLowDisplay % 100))/100;
      int ones = counterLowDisplay % 10;
      int tens = ((counterLowDisplay - (hundreds * 100)) - ones)/10;
      
      // Serial.print("Low Hundreds: ");
      // Serial.print(hundreds);
      // Serial.print(", tens: ");
      // Serial.print(tens);
      // Serial.print(", ones: ");
      // Serial.println(ones);
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
    }*/
}

void updateDelayBarsDisplay(){
  if(delayHigh >= 32){
    dma_display->writeFillRect(0, 0, 64, 5, colorLight);
    dma_display->writeFillRect(0, 27, 64, 5, colorLight);
  }
  else if(delayHigh > 0){
    dma_display->writeFillRect(0, 0, delayHigh, 5, colorDark);
  }

  if(delayLow >= 32){
    dma_display->writeFillRect(0, 59, 32, 5, colorLight);
  }
  else if(delayLow > 0){
    dma_display->writeFillRect(0, 59, delayLow, 5, colorDark);
  }
}

void setup() {
  Serial.begin(115200);
  
  initDisplay();
  buttonSetup();
  timerISRInit();

  //bars 
  //dma_display->writeFillRect(0, 0, 64, 5, colorLight);
  // dma_display->writeFillRect(0, 27, 64, 5, colorLight);
}

void loop() {
  /*
  Serial.print("Counter High: ");
  Serial.print(counterHigh);
  Serial.print(", Counter Low: ");
  Serial.print(counterLow);
  Serial.print(", Counter Three: ");
  Serial.print(counterThree);
*/

  points = counterLow+counterHigh+counterThree-3; 

  //Serial.print(", points: ");
  //Serial.println(points);

  ButtonInterruptFunction1(); 
  ButtonInterruptFunction2(); 
  ButtonInterruptFunction3(); 
  ButtonInterruptFunction4(); 
  ButtonInterruptFunction5();
  ButtonInterruptFunction6();

  if(points!=oldPoints){
    dma_display->clearScreen();
    // updateDelayBarsDisplay(); 
    updatePointsDisplay();
    oldPoints = points; 
  }

  

  //delay(200);
}
