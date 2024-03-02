//Display Refresh is on a ~ 60Hz timer interrupt. It grabs data from global variables and redraws the display if needed

#define REV_B01

#include <ESP32-HUB75-MatrixPanel-I2S-DMA.h>
#include "LEDMatrixConfig.h"
MatrixPanel_I2S_DMA *dma_display = nullptr;

#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include <Wire.h>
Adafruit_MPU6050 mpu;

hw_timer_t * timer = NULL;

sensors_event_t a, g, temp;

int8_t angleBuffer[8];

#include <Adafruit_NeoPixel.h>
Adafruit_NeoPixel pixels(1, NEOPIXEL_DATA, NEO_GRB + NEO_KHZ800);

void initNeopixel(){
  pinMode(NEOPIXEL_POWER, OUTPUT);
  digitalWrite(NEOPIXEL_POWER, HIGH); //turn on neopixel
  pixels.begin();
  pixels.clear();
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

void updateDisplay(){
  int32_t angle = 0;
  for(int counter = 0; counter < 8; counter++){
    angle += angleBuffer[counter];
  }
  angle = angle >> 3; //divide by 8
  dma_display->clearScreen();
  dma_display->setCursor(0, 0);
  dma_display->setTextSize(2);
  dma_display->println(angle);
}

void IRAM_ATTR timerISR() {

}

void timerISRInit(){
  timer = timerBegin(1, 80, true); // Timer 0, divider 80
  timerAttachInterrupt(timer, &timerISR, true); // Attach ISR
  timerAlarmWrite(timer, 15625, true); // updates 32 times in a 500ms period
  timerAlarmEnable(timer); // Enable the timer
}

void initMPU(){
  if (!mpu.begin()) {
  Serial.println("Failed to find MPU6050 chip");
  pixels.setPixelColor(0, pixels.Color(255, 0, 0));
  pixels.show(); 
  dma_display->clearScreen();
  dma_display->setCursor(0, 0);
  dma_display->setTextSize(1);
  dma_display->println("No Sensor Found!");
  while (1) {
    delay(10);
    }
  }
  Serial.println("MPU6050 Found!");
  pixels.setPixelColor(0, pixels.Color(0, 255, 0));
  pixels.show(); 
  mpu.setAccelerometerRange(MPU6050_RANGE_2_G);
  mpu.setFilterBandwidth(MPU6050_BAND_260_HZ);
}

void setup() {
  Serial.begin(115200);
  delay(10);
  initNeopixel();
  initDisplay();
  initMPU();
  timerISRInit();
}

void loop() {
  uint16_t tilt_averaged = 0;
  for(int sample = 0; sample < 8; sample++){
  mpu.getEvent(&a, &g, &temp);
  uint8_t tilt = (acos(a.acceleration.z / sqrt(a.acceleration.x * a.acceleration.x + a.acceleration.y * a.acceleration.y + a.acceleration.z * a.acceleration.z)) * 180 / (3.14159));
  angleBuffer[sample] = tilt;
  tilt_averaged += tilt;
  delay(10);
  }
  tilt_averaged = tilt_averaged >> 3; //divide by 8
  dma_display->clearScreen();
  dma_display->setCursor(0, 0);
  dma_display->setTextSize(1);
  dma_display->print("X:");
  dma_display->println(a.acceleration.x/8);
  dma_display->print("Y: ");
  dma_display->println(a.acceleration.y/8);
  dma_display->print("Z: ");
  dma_display->println(a.acceleration.z/8);
  dma_display->print("tilt: ");
  dma_display->print(tilt_averaged);
  if(a.acceleration.y < 0){
    dma_display->print("R");
  }
  else{
    dma_display->print("L");
  }
}
