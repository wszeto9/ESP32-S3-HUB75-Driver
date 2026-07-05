#define REV_B01

#include <ESP32-HUB75-MatrixPanel-I2S-DMA.h>
#include "LEDMatrixConfig.h"
#include "MBTA_API_Config.h"
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <NTPClient.h>
#include <string.h>
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
const long gmtOffset_sec = -4 * 60 * 60; // Your GMT offset in seconds
const int daylightOffset_sec = 3600; // Daylight offset in seconds (1 hour)
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, ntpServer, gmtOffset_sec, daylightOffset_sec);

MatrixPanel_I2S_DMA *dma_display = nullptr;

hw_timer_t * timer = NULL;

HTTPClient httpMBTA;

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
  //drawXbm565(40, 0, 6, 7, bus_bits, dma_display->color565(MBTAColor[0],MBTAColor[1],MBTAColor[2]));
  drawXbm565(0, 56, 7, 7, clock_circle_bits, dma_display->color565(MBTAColor[0],MBTAColor[1],MBTAColor[2]));
  drawXbm565(0, 56, 7, 7, clock_hands_bits, dma_display->color565(MITColor[0],MITColor[1],MITColor[2]));

}

void refreshScreen(){
  dma_display->clearScreen();
  dma_display->setCursor(0, 0);    // start at top left, with 8 pixel of spacing
  for(int i = 0; i < 8; i++){
    if(i == 7){
      dma_display->setCursor(5, 56);
    }
    int redraw = 0;
    if(i == 4){
      String text = DisplayBuffer[i];
      if(text[7] == 'i'){
        drawText(DisplayBuffer[i].substring(0,5), LineColorsRed[i],LineColorsGreen[i],LineColorsBlue[i]);
        dma_display->setCursor(33, 32);
        drawText("min", LineColorsRed[i],LineColorsGreen[i],LineColorsBlue[i]);
        redraw = 1;
      }
    }
    if(i == 5){
      String text = DisplayBuffer[i];
      if(text[7] == 'i'){
        drawText(DisplayBuffer[i].substring(0,5), LineColorsRed[i],LineColorsGreen[i],LineColorsBlue[i]);
        dma_display->setCursor(33, 40);
        drawText("min", LineColorsRed[i],LineColorsGreen[i],LineColorsBlue[i]);
        redraw = 1;
      }
    }
    if(!redraw){
      drawText(DisplayBuffer[i], LineColorsRed[i],LineColorsGreen[i],LineColorsBlue[i]);
    }
  }
}

void timerISRInit() {
  timer = timerBegin(1000000);           // 1 MHz timer tick = 1 us
  timerAttachInterrupt(timer, &timerISR);
  timerAlarm(timer, 1000000, true, 0);   // 1s period, autoreload, unlimited reloads
}

void drawText(String text, uint8_t r, uint8_t g, uint8_t b)
{
  dma_display->setTextSize(1);     // size 1 == 8 pixels high
  dma_display->setTextWrap(false); // Don't wrap at end of line - will do ourselves
  dma_display->setTextColor(dma_display->color565(r,g,b));
  dma_display->println(text);
}

void initMatrix(){

  DisplayBuffer[0] = "66 Harvard";
  DisplayBuffer[1] = "";
  DisplayBuffer[2] = "";
  DisplayBuffer[3] = "";
  DisplayBuffer[4] = "";
  DisplayBuffer[5] = "";
  DisplayBuffer[6] = "";
  DisplayBuffer[7] = "";
  for (int i = 0; i < 8; i++) {
    LineColorsRed[i] = DefaultTextColor[0];
    LineColorsGreen[i] = DefaultTextColor[1];
    LineColorsBlue[i] = DefaultTextColor[2];
  }

  HUB75_I2S_CFG::i2s_pins _pins={R1_PIN, G1_PIN, B1_PIN, R2_PIN, G2_PIN, B2_PIN, A_PIN, B_PIN, C_PIN, D_PIN, E_PIN, LAT_PIN, OE_PIN, CLK_PIN};
  HUB75_I2S_CFG mxconfig(PANEL_RES_X, PANEL_RES_Y, PANEL_CHAIN, _pins);
  //mxconfig.gpio.e = E_PIN;
  mxconfig.clkphase = false;
  dma_display = new MatrixPanel_I2S_DMA(mxconfig);
  dma_display->setRotation(displayRotation);
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
}

struct MBTAPredictionCandidate {
  int minutesAway;
  String displayText;
};

int getPredictionMinutesAway(String eventTime) {
  if (eventTime.length() < 16) {
    return -1;
  }

  int eventMinutes = eventTime.substring(11, 13).toInt() * 60 + eventTime.substring(14, 16).toInt();
  int nowMinutes = timeClient.getHours() * 60 + timeClient.getMinutes();
  int minutesAway = eventMinutes - nowMinutes;

  // Handle predictions after midnight while current time is before midnight.
  if (minutesAway < -12 * 60) {
    minutesAway += 24 * 60;
  }

  // Drop stale predictions from earlier today. Keep buses that are arriving now.
  if (minutesAway < -1) {
    return -1;
  }
  if (minutesAway < 0) {
    minutesAway = 0;
  }

  return minutesAway;
}

void sortPredictionsByArrival(int *minutesAway, String *displayText, int count) {
  for (int i = 0; i < count - 1; i++) {
    for (int j = i + 1; j < count; j++) {
      if (minutesAway[j] < minutesAway[i]) {
        int tmpMinutes = minutesAway[i];
        minutesAway[i] = minutesAway[j];
        minutesAway[j] = tmpMinutes;

        String tmpText = displayText[i];
        displayText[i] = displayText[j];
        displayText[j] = tmpText;
      }
    }
  }
}

void getUpdateMBTAtimes(){
  int httpResponseCode  = httpMBTA.GET();
  if (httpResponseCode  > 0) {
    String payload = httpMBTA.getString();
    Serial.println(payload);

    DynamicJsonDocument doc(8192);
    DeserializationError error = deserializeJson(doc, payload);
    if (error) {
      Serial.print(F("deserializeJson() failed: "));
      Serial.println(error.f_str());
      DisplayBuffer[1] = "JSON Err";
      DisplayBuffer[2] = "";
      DisplayBuffer[3] = "";
      return;
    }

    DisplayBuffer[0] = "66 Harvard";
    DisplayBuffer[1] = "No bus";
    DisplayBuffer[2] = "";
    DisplayBuffer[3] = "";
    DisplayBuffer[4] = "";
    DisplayBuffer[5] = "";
    DisplayBuffer[6] = "";

    int candidateMinutes[10];
    String candidateDisplayText[10];
    int candidateCount = 0;

    for (JsonObject prediction : doc["data"].as<JsonArray>()) {
      const char *routeId = prediction["relationships"]["route"]["data"]["id"] | "";
      const char *stopId = prediction["relationships"]["stop"]["data"]["id"] | "";
      const char *revenue = prediction["attributes"]["revenue"] | "";
      const char *status = prediction["attributes"]["status"] | "";

      if (strcmp(routeId, MBTA_ROUTE_ID) != 0 || strcmp(stopId, MBTA_STOP_ID) != 0) {
        continue;
      }
      if (strcmp(revenue, "REVENUE") != 0) {
        continue;
      }
      if (strcmp(status, "CANCELLED") == 0 || strcmp(status, "SKIPPED") == 0) {
        continue;
      }

      String eventTime = prediction["attributes"]["arrival_time"] | "";
      if (eventTime.length() == 0 || eventTime == "null" || eventTime == "None") {
        eventTime = prediction["attributes"]["departure_time"] | "";
      }

      int minutesAway = getPredictionMinutesAway(eventTime);
      if (minutesAway < 0) {
        continue;
      }

      candidateMinutes[candidateCount] = minutesAway;
      if (minutesAway == 0) {
        candidateDisplayText[candidateCount] = "ARRIVE";
      } else {
        candidateDisplayText[candidateCount] = String(minutesAway) + " min";
      }

      Serial.print("candidate ");
      Serial.print(candidateCount);
      Serial.print(": ");
      Serial.print(eventTime);
      Serial.print(" -> ");
      Serial.print(minutesAway);
      Serial.println(" min");

      candidateCount++;
      if (candidateCount >= 10) {
        break;
      }
    }

    sortPredictionsByArrival(candidateMinutes, candidateDisplayText, candidateCount);

    for (int i = 0; i < candidateCount && i < 4; i++) {
      int line = 1 + i;
      DisplayBuffer[line] = candidateDisplayText[i];
      LineColorsRed[line] = MBTAColor[0];
      LineColorsGreen[line] = MBTAColor[1];
      LineColorsBlue[line] = MBTAColor[2];

      Serial.print("display ");
      Serial.print(i);
      Serial.print(": ");
      Serial.print(candidateMinutes[i]);
      Serial.println(" min");
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
  Serial.println(timeClient.getFormattedTime().substring(0,5));
  DisplayBuffer[7] = " now:" + timeClient.getFormattedTime().substring(0,5);
  LineColorsRed[7] = DefaultTextColor[0];
  LineColorsGreen[7] = DefaultTextColor[1];
  LineColorsBlue[7] = DefaultTextColor[2];
  delay(1000 * 10);
}