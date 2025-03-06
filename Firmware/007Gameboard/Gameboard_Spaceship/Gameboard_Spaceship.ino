//Display Refresh is on a ~ 60Hz timer interrupt. It grabs data from global variables and redraws the display if needed
#define REV_B01

#define YELLOWGREENSIDE
//#define REDBLUESIDE

#ifdef REDBLUESIDE
#define ColorLeft 0xF800
#define ColorRight 0x001F
#endif

#ifdef YELLOWGREENSIDE
#define ColorRight 0xfde0
#define ColorLeft 0x25a0
#endif

#include <esp_now.h>
#include <WiFi.h>
#include "VisualAssets.h"
#include <ESP32-HUB75-MatrixPanel-I2S-DMA.h>
#include "LEDMatrixConfig.h"
MatrixPanel_I2S_DMA *dma_display = nullptr;

#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include <Wire.h>
Adafruit_MPU6050 mpu;

hw_timer_t * timer = NULL;

sensors_event_t a, g, temp;
uint8_t avg_filt = 16;
float angleBuffer[16];

#include <Adafruit_NeoPixel.h>
Adafruit_NeoPixel pixels(1, NEOPIXEL_DATA, NEO_GRB + NEO_KHZ800);

uint8_t displayDebug = 0;

#include "esp32-hal-timer.h" 
hw_timer_t * timerFlashlight = NULL;
uint8_t flashlightOn = 0;
uint8_t flashlightBrightness = 100;
float Multiplier = 1;
float OldMultiplier;

unsigned long timeElapsed = 0;

// Structure example to receive data
// Must match the sender structure
typedef struct struct_message {
    char a[32];
    uint8_t b;
    int c;
} struct_message;

// Create a struct_message called myData
struct_message myData;

uint8_t displayText = 0;


void drawXbm565(int x, int y, int width, int height, const char *xbm, uint16_t color = 0xffff) {
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

void initMPU(){
  if (!mpu.begin()) {
  Serial.println("Failed to find MPU6050 chip");
  pixels.setPixelColor(0, pixels.Color(255, 0, 0));
  pixels.show(); 
  dma_display->clearScreen();
  while (!mpu.begin()) {
    drawXbm565(0,0,64,32,NoAccelWhite);
    delay(1000);
    drawXbm565(0,0,64,32,NoAccelRed, 0xf800);
    delay(1000);
    dma_display->clearScreen();
    }
  }
  Serial.println("MPU6050 Found!");
  pixels.setPixelColor(0, pixels.Color(0, 255, 0));
  pixels.show(); 
  mpu.setAccelerometerRange(MPU6050_RANGE_2_G);
}

void IRAM_ATTR TopButtonISR(){
  displayDebug = 1;
}

void IRAM_ATTR ButtomButtonISR(){
  displayDebug = 0;
}

void initButtons(){
  pinMode(TOP_BUTTON, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(TOP_BUTTON), TopButtonISR, FALLING);
  pinMode(BOTTOM_BUTTON, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(BOTTOM_BUTTON), ButtomButtonISR, FALLING);
}

void initFlashlight(){
  pinMode(flashlightLeft, OUTPUT);
  pinMode(flashlightRight, OUTPUT);
  xTaskCreatePinnedToCore(
    FlashlightBlink,          // Function to implement the task
    "FlashlightTask",    // Name of the task
    2048,                // Stack size of the task
    NULL,                // Parameter of the task
    1,                   // Priority of the task
    NULL,                // Task handle
    0);                    // Core where the task should run
}

void InitESPNow() {
  WiFi.disconnect();
  if (esp_now_init() == ESP_OK) {
    Serial.println("ESPNow Init Success");
  }
  else {
    Serial.println("ESPNow Init Failed");
    // Retry InitESPNow, add a counte and then restart?
    // InitESPNow();
    // or Simply Restart
    ESP.restart();
  }
}

void configDeviceAP() {
  const char *SSID = "WifiAP";
  bool result = WiFi.softAP(SSID, "password", 1, 0);
  if (!result) {
    Serial.println("AP Config failed.");
  } else {
    Serial.println("AP Config Success. Broadcasting with AP: " + String(SSID));
    Serial.print("AP CHANNEL "); Serial.println(WiFi.channel());
  }
  Serial.print("AP MAC: "); Serial.println(WiFi.softAPmacAddress());
}

void OnDataRecv(const uint8_t * mac, const uint8_t *incomingData, int len) {
  displayText = len;
  memcpy(&myData, incomingData, sizeof(myData));
  Serial.print("Bytes received: ");
  Serial.println(len);
  Serial.print("Char: ");
  Serial.println(myData.a);

  // dma_display -> clearScreen();

  // dma_display->setTextSize(1);
  // dma_display->setCursor(0, 0);
  // dma_display->println("Hello!");
  // delay(10000);
}

void setup() {
  Serial.begin(115200);
  delay(10);
  initNeopixel();
  initDisplay();
  initMPU();
  initButtons();
  initFlashlight();

  WiFi.mode(WIFI_STA);
    // Init ESP-NOW
  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }
  // Once ESPNow is successfully Init, we will register for recv CB to
  // get recv packer info
  esp_now_register_recv_cb(OnDataRecv);

}

void showUpdateDebugDisplay(float xaccel, float yaccel, float zaccel, float tilt){
  dma_display->clearScreen();
  dma_display->setCursor(0, 0);
  dma_display->setTextSize(1);
  dma_display->print("X:");
  dma_display->println(xaccel);
  dma_display->print("Y:");
  dma_display->println(yaccel);
  dma_display->print("Z:");
  dma_display->println(zaccel);
  dma_display->print("tilt:");
  dma_display->print((float) round(tilt * 10)/10);
  if(xaccel < 0){
    dma_display->print("R");
  }
  else{
    dma_display->print("L");
  }
  if(tilt > 16){
    dma_display->setCursor(40, 16);
    dma_display->print("2.0x");
  }
  else if(tilt > 12){
    dma_display->setCursor(40, 16);
    dma_display->print("1.8x");
  }
  else if(tilt > 8){
    dma_display->setCursor(40, 16);
    dma_display->print("1.5x");
  }
  else if(tilt > 4){
    dma_display->setCursor(40, 16);
    dma_display->print("1.2x");
  }
  else{
    dma_display->setCursor(40, 16);
    dma_display->print("1.0x");
  }
}

void showUpdateDisplay(float xaccel, float yaccel, float zaccel, float tilt){
  dma_display->clearScreen();
  dma_display->fillRect(22, 23, 3, 3, 0xFFFF );
  dma_display->setCursor(4, 5);
  dma_display->setTextSize(3);
  if(tilt > 16){
    dma_display->print("2");
  }
  else{
    dma_display->print("1");
  }
  dma_display->setCursor(28, 5);
  if(tilt > 16){
    dma_display->print("0x");
    Multiplier = 2;
  }
  else if(tilt > 12){
    dma_display->print("8x");
    Multiplier = 1.8;
  }
  else if(tilt > 8){
    dma_display->print("5x");
    Multiplier = 1.5;
  }
  else if(tilt > 4){
    dma_display->print("2x");
    Multiplier = 1.3;
  }
  else{
    dma_display->print("0x");
    Multiplier = 1;
  }
  if(tilt > 1){
    if(xaccel > 0){
      dma_display->fillRect(32, 0, tilt * 32 / 20, 2, ColorRight);
      dma_display->fillRect(32, 30, tilt * 32 / 20, 2, ColorRight);
      if(tilt > 20){
        dma_display->drawRect(0, 0, 64, 32, ColorRight);
        dma_display->drawRect(1, 1, 62, 30, ColorRight);
      }
    }
    else{
      Multiplier = -Multiplier;
      dma_display->fillRect(33 - tilt * 32 / 20, 0, tilt * 32 / 20, 2, ColorLeft);
      dma_display->fillRect(33 - tilt * 32 / 20, 30, tilt * 32 / 20, 2, ColorLeft); 
      if(tilt > 20){
        dma_display->drawRect(0, 0, 64, 32, ColorLeft);
        dma_display->drawRect(1, 1, 62, 30, ColorLeft);
      }
    }
  }

}

void loop() {
  if(displayText){
    dma_display -> clearScreen();
    dma_display->setTextSize(2);
    dma_display->setBrightness(255);
    dma_display->setTextColor(0xFFFFFF, 0x000000);
    dma_display->fillRect(0, 0, 64, 6, myData.c);
    dma_display->fillRect(0, 26, 64, 6, myData.c);
    while(1){
      for(int i = 64; i > - (myData.b * 12); i+= -1){
        dma_display->setCursor(i,8);
        dma_display->println(myData.a);
        delay(100);
      }
    }
  }

  float tilt_averaged = 0;
  for(int sample = 0; sample < avg_filt; sample++){
  mpu.getEvent(&a, &g, &temp);
  float tilt = (acos(a.acceleration.z / sqrt(a.acceleration.x * a.acceleration.x + a.acceleration.y * a.acceleration.y + a.acceleration.z * a.acceleration.z)) * 180 / (3.14159));
  angleBuffer[sample] = tilt;
  tilt_averaged += tilt;

  delay(1);
  }
  tilt_averaged = tilt_averaged / avg_filt; //divide by 8
  
  if(displayDebug){
    showUpdateDebugDisplay(a.acceleration.x / 8, a.acceleration.y / 8, a.acceleration.z / 8, tilt_averaged);
  }
  else{
    showUpdateDisplay(a.acceleration.x / 8, a.acceleration.y / 8, a.acceleration.z / 8, tilt_averaged);
  }
  
  Serial.print("Brightness: ");
  Serial.print(analogRead(ALS_PIN));
  Serial.print(", Multiplier: ");
  Serial.print(Multiplier);
  Serial.print(", X: ");
  Serial.print(a.acceleration.x/8);
  Serial.print(", Y: ");
  Serial.print(a.acceleration.y/8);
  Serial.print(", Z: ");
  Serial.print(a.acceleration.z/8);
  Serial.print(", tilt: ");
  Serial.print(tilt_averaged);
  if(a.acceleration.x < 0){
    Serial.println("R");
  }
  else{
    Serial.println("L");
  }
  //dma_display->setBrightness(map(analogRead(ALS_PIN), 0, 4095, BRIGHTNESS,0));
}

void FlashlightBlink( void* ){
  while(1){
    if(Multiplier > 1){
      analogWrite(flashlightLeft, flashlightBrightness);

      for(int i = 0; i < 10; i++){
        if(Multiplier > 1){
          delay(100 / (abs(Multiplier * Multiplier * Multiplier)));
        }
      }

      analogWrite(flashlightLeft, 0);
      analogWrite(flashlightRight, 0);

      for(int i = 0; i < 10; i++){
        if(Multiplier > 1){
          delay(100 / (abs(Multiplier * Multiplier * Multiplier)));
        }
      }

    }
    else if(Multiplier < -1){

      analogWrite(flashlightRight, flashlightBrightness);

      for(int i = 0; i < 10; i++){
        if(Multiplier < -1){
          delay(100 / (abs(Multiplier * Multiplier * Multiplier)));
        }
      }

      analogWrite(flashlightLeft, 0);
      analogWrite(flashlightRight, 0);

      for(int i = 0; i < 10; i++){
        if(Multiplier < -1){
          delay(100 / (abs(Multiplier * Multiplier * Multiplier)));
        }
      }

    }
    delay(10);
  }
}
