#include <ESP32-HUB75-MatrixPanel-I2S-DMA.h>
#include "LEDMatrixConfig.h"
#include "MBTA_API_Config.h"
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <NTPClient.h>
#include "logos.h"
#include "WifiPassword.h" //defines const char *password = "password here";

#define BRIGHTNESS 60

String DisplayBuffer[8];
uint8_t LineColorsRed[8];
uint8_t LineColorsGreen[8];
uint8_t LineColorsBlue[8];

uint8_t DefaultTextColor[3] = {186, 85, 211};
uint8_t MBTAColor[3] = {255,255,0};
int MBTAArrivalTimes[100];
uint8_t MITColor[3] = {255,0,44};

const char *ntpServer = "pool.ntp.org";
const long gmtOffset_sec = -5 * 60 * 60; // Your GMT offset in seconds
const int daylightOffset_sec = 3600; // Daylight offset in seconds (1 hour)
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, ntpServer, gmtOffset_sec, daylightOffset_sec);

MatrixPanel_I2S_DMA *dma_display = nullptr;

hw_timer_t * timer = NULL;

HTTPClient httpMBTA;
HTTPClient httpMIT;
HTTPClient httpMITMorning;

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

void IRAM_ATTR timerISR() {
  //Serial.println("Updating the display with interrupts! Here's the display stuffs"); // Print to serial
  refreshScreen();
  drawXbm565(57, 8, 7, 7, MBTA_bits, dma_display->color565(MBTAColor[0],MBTAColor[1],MBTAColor[2]));
  drawXbm565(57, 16, 7, 7, MBTA_bits, dma_display->color565(MBTAColor[0],MBTAColor[1],MBTAColor[2]));
  drawXbm565(57, 24, 7, 7, MBTA_bits, dma_display->color565(MBTAColor[0],MBTAColor[1],MBTAColor[2]));
  drawXbm565(53, 33, 11, 7, MIT_bits, dma_display->color565(MITColor[0],MITColor[1],MITColor[2]));
}

void refreshScreen(){
  dma_display->clearScreen();
  dma_display->setCursor(0, 0);    // start at top left, with 8 pixel of spacing
  for(int i = 0; i < 8; i++){
    drawText(DisplayBuffer[i], LineColorsRed[i],LineColorsGreen[i],LineColorsBlue[i]);
    //Serial.println(DisplayBuffer[i]);
  }
  //Serial.println(" "); 
}

void timerISRInit(){
  timer = timerBegin(0, 80, true); // Timer 0, divider 80
  timerAttachInterrupt(timer, &timerISR, true); // Attach ISR
  timerAlarmWrite(timer, 1000000, true); // 1s period
  timerAlarmEnable(timer); // Enable the timer
}

void drawText(String text, uint8_t r, uint8_t g, uint8_t b)
{
  dma_display->setTextSize(1);     // size 1 == 8 pixels high
  dma_display->setTextWrap(false); // Don't wrap at end of line - will do ourselves
  dma_display->setTextColor(dma_display->color565(r,g,b));
  dma_display->println(text);
}

void initMatrix(){

  DisplayBuffer[0] = "Buses at: ";
  DisplayBuffer[5] = "9 min walk";
  DisplayBuffer[6] = "Time now:";

  LineColorsRed[0] = DefaultTextColor[0];
  LineColorsGreen[0] = DefaultTextColor[1];
  LineColorsBlue[0] = DefaultTextColor[2];
  LineColorsRed[5] = DefaultTextColor[0];
  LineColorsGreen[5] = DefaultTextColor[1];
  LineColorsBlue[5] = DefaultTextColor[2];
  LineColorsRed[6] = DefaultTextColor[0];
  LineColorsGreen[6] = DefaultTextColor[1];
  LineColorsBlue[6] = DefaultTextColor[2];

  HUB75_I2S_CFG::i2s_pins _pins={R1_PIN, G1_PIN, B1_PIN, R2_PIN, G2_PIN, B2_PIN, A_PIN, B_PIN, C_PIN, D_PIN, E_PIN, LAT_PIN, OE_PIN, CLK_PIN};
  HUB75_I2S_CFG mxconfig(PANEL_RES_X, PANEL_RES_Y, PANEL_CHAIN, _pins);
  mxconfig.gpio.e = E_PIN;
  mxconfig.clkphase = false;
  dma_display = new MatrixPanel_I2S_DMA(mxconfig);
  dma_display->begin();
  dma_display->setBrightness8(BRIGHTNESS); //0-255
  dma_display->clearScreen();
}

void initWifi(){
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
    dma_display->clearScreen();
    drawText("Connecting", 255,0,0);
  }
  Serial.println("Connected to WiFi");
  drawText("Connected", 0,255,0);
  delay(1000);
  httpMBTA.begin(apiEndpointMBTA);
  httpMIT.begin(apiEndpointMIT);
  httpMITMorning.begin(apiEndpointMITMorning);
}

void getUpdateMITShuttleTimes(){
  int httpResponseCode = httpMIT.GET();
  if(httpResponseCode > 0){
    String payload = httpMIT.getString();
    Serial.print("MIT Data: ");
    Serial.println(payload);
    const size_t capacity2 = JSON_OBJECT_SIZE(10) + 300;
    DynamicJsonDocument doc(capacity2);
    DeserializationError error2 = deserializeJson(doc, payload);
    if(error2){
      Serial.print(F("deserializeJson() failed: "));
      Serial.println(error2.f_str());
      return;
    }
    JsonObject predictions = doc["ETAs"]["992"][0];
    String ArrivalTime = predictions["eta"].as<String>();
    if(ArrivalTime.substring(0,9) == "less than"){
      ArrivalTime = "<1 min";
    }
    DisplayBuffer[4] = ArrivalTime;
    LineColorsRed[4] = 255;
    LineColorsBlue[4] = 44;
  }
  if(timeClient.getHours() < 1 && timeClient.getHours() < 18){
    DisplayBuffer[4] = "No.";
  }
  
}

void getUpdateMITShuttleMorningTimes(){
  int httpResponseCode = httpMITMorning.GET();
  if(httpResponseCode > 0){
    String payload = httpMITMorning.getString();
    Serial.print("MIT Morning Shuttle Data: ");
    Serial.println(payload);
    const size_t capacity2 = JSON_OBJECT_SIZE(10) + 300;
    DynamicJsonDocument doc(capacity2);
    DeserializationError error2 = deserializeJson(doc, payload);
    if(error2){
      Serial.print(F("deserializeJson() failed: "));
      Serial.println(error2.f_str());
      return;
    }
    JsonObject predictions = doc["ETAs"]["992"][0];
    String ArrivalTime = predictions["eta"].as<String>();
    if(ArrivalTime.substring(0,9) == "less than"){
      ArrivalTime = "<1 min";
    }
    DisplayBuffer[4] = ArrivalTime;
    LineColorsRed[4] = 255;
    LineColorsBlue[4] = 44;
  }
  if(timeClient.getHours() < 8 || timeClient.getHours() >= 23){
    DisplayBuffer[4] = "No.";
  }
  
}

void getUpdateMBTAtimes(){
  for(int i = 0; i < 100; i++){
    MBTAArrivalTimes[i] = 9999;
  }
  int httpResponseCode  = httpMBTA.GET();
  // Check for a successful response
  if (httpResponseCode  > 0) {
    String payload = httpMBTA.getString();
    Serial.println(payload);
    const size_t capacity = JSON_OBJECT_SIZE(10) + 300;
    DynamicJsonDocument doc(capacity);
    DeserializationError error = deserializeJson(doc, payload);
    if (error) {
      Serial.print(F("deserializeJson() failed: "));
      Serial.println(error.f_str());
      return;
    }
    for(int idx = 0; idx < 100; idx++){
      JsonObject prediction = doc["data"][idx];
      if(prediction["attributes"].containsKey("arrival_time") && prediction["attributes"]["arrival_time"] != "None"){
        if(prediction["relationships"]["stop"]["data"]["id"] == "95"){
          Serial.print("Index ");
          Serial.print(idx);
          Serial.print(" is for stop 95 (Beacon St.) Arrival time: ");
          String arrivalTime = prediction["attributes"]["arrival_time"].as<String>();
          String ParsedTime = arrivalTime.substring(11, 16);
          Serial.println(ParsedTime);
          if(arrivalTime.substring(13,14) == ":"){
            MBTAArrivalTimes[idx] = (arrivalTime.substring(11,13).toInt() - timeClient.getHours()) * 60 + arrivalTime.substring(14,16).toInt() - timeClient.getMinutes();
            if(MBTAArrivalTimes[idx] < 0){
              MBTAArrivalTimes[idx] = MBTAArrivalTimes[idx] + 24 * 60; //hour wraparound
            }
            Serial.println(MBTAArrivalTimes[idx]);
          }
        }
      }
    }
    for(int line = 1; line < 4; line++){
      int ShortestArrivalTime = 999;
      int ShortestArrivalTimeIndex;
      for(int i = 0; i < 100; i++){
        if(MBTAArrivalTimes[i] < ShortestArrivalTime && ShortestArrivalTime > 0){
          ShortestArrivalTime = MBTAArrivalTimes[i];
          ShortestArrivalTimeIndex = i;
          Serial.print("Line ");
          Serial.print(i);
          Serial.print(" has the shortest time of ");
          Serial.println(MBTAArrivalTimes[i]);
        }
      }
      if(ShortestArrivalTime < 900){
        DisplayBuffer[line] = String(ShortestArrivalTime) + " min";
        LineColorsRed[line] = MBTAColor[0];
        LineColorsGreen[line] = MBTAColor[1];
        LineColorsBlue[line] = MBTAColor[2];
        MBTAArrivalTimes[ShortestArrivalTimeIndex] = 9999;
      }
      else{
        DisplayBuffer[line] = "No. ";
        LineColorsRed[line] = MBTAColor[0];
        LineColorsGreen[line] = MBTAColor[1];
        LineColorsBlue[line] = MBTAColor[2];
      }
    }
  }
  else {
    Serial.print("Error on HTTP request. Code: ");
    Serial.println(httpResponseCode);
    DisplayBuffer[1] = "HTTP Error";
    LineColorsRed[1] = 255;
    LineColorsGreen[1] = 0;
    LineColorsBlue[1] = 0;
    DisplayBuffer[2] = String(httpResponseCode);
    LineColorsRed[2] = 255;
    LineColorsGreen[2] = 0;
    LineColorsBlue[2] = 0;
  }
}

void setup() {
  Serial.begin(1000000);
  initMatrix();
  initWifi();
  timerISRInit();
  timeClient.begin();
  timeClient.update();
}

void loop() {
  timeClient.update();
  getUpdateMBTAtimes();
  if(timeClient.getHours() < 18){
    getUpdateMITShuttleMorningTimes();
  }
  else{
    getUpdateMITShuttleTimes();
  }
  Serial.println(timeClient.getFormattedTime().substring(0,5));
  DisplayBuffer[7] = timeClient.getFormattedTime().substring(0,5);
  LineColorsRed[7] = DefaultTextColor[0];
  LineColorsGreen[7] = DefaultTextColor[1];
  LineColorsBlue[7] = DefaultTextColor[2];
  delay(1000 * 10); // 1 minute
}
