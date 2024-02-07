#include <ESP32-HUB75-MatrixPanel-I2S-DMA.h>
#include "LEDMatrixConfig.h"
#include "MBTA_API_Config.h"
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <NTPClient.h>

const char *ntpServer = "pool.ntp.org";
const long gmtOffset_sec = -5 * 60 * 60; // Your GMT offset in seconds
const int daylightOffset_sec = 3600; // Daylight offset in seconds (1 hour)
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, ntpServer, gmtOffset_sec, daylightOffset_sec);

MatrixPanel_I2S_DMA *dma_display = nullptr;

void drawText(String text)
{
  dma_display->setTextSize(1);     // size 1 == 8 pixels high
  dma_display->setTextWrap(false); // Don't wrap at end of line - will do ourselves
  dma_display->setTextColor(dma_display->color444(15,15,15));
  dma_display->println(text);
}

void setup() {
  Serial.begin(1000000);


  HUB75_I2S_CFG::i2s_pins _pins={R1_PIN, G1_PIN, B1_PIN, R2_PIN, G2_PIN, B2_PIN, A_PIN, B_PIN, C_PIN, D_PIN, E_PIN, LAT_PIN, OE_PIN, CLK_PIN};
  HUB75_I2S_CFG mxconfig(PANEL_RES_X, PANEL_RES_Y, PANEL_CHAIN, _pins);
  mxconfig.gpio.e = E_PIN;
  mxconfig.clkphase = false;
  dma_display = new MatrixPanel_I2S_DMA(mxconfig);
  dma_display->begin();
  dma_display->setBrightness8(60); //0-255
  dma_display->clearScreen();

  
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
  delay(1000);
    Serial.println("Connecting to WiFi...");
    dma_display->clearScreen();
    drawText("Connecting");
  }
  Serial.println("Connected to WiFi");
  dma_display->clearScreen();
  drawText("Connected");
  delay(1000);

  timeClient.begin();
  timeClient.update();

}

void loop() {


     // Create an HTTP client object
  HTTPClient http;

  // Specify the API endpoint
  http.begin(apiEndpoint);

  // Make the request
  int httpResponseCode  = http.GET();

  dma_display->clearScreen();
  dma_display->setCursor(0, 0);
  drawText("1 Bus at: ");

  // Check for a successful response
  if (httpResponseCode  > 0) {
    String payload = http.getString();
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
        Serial.print("Index ");
        Serial.print(idx);
        Serial.println(" has arrival time");
        if(prediction["relationships"]["stop"]["data"]["id"] == "95"){
          Serial.print("Index ");
          Serial.print(idx);
          Serial.print(" is for stop 95 (Beacon St.) Arrival time: ");
          String arrivalTime = prediction["attributes"]["arrival_time"].as<String>();
          String ParsedTime = arrivalTime.substring(11, 16);
          Serial.println(ParsedTime);
          if(arrivalTime.substring(13, 14) == ":"){
            drawText(ParsedTime);
          }
        }
      }
    }
  }
  else {
    Serial.print("Error on HTTP request. Code: ");
    Serial.println(httpResponseCode);
  }

  // Close the connection
  http.end();
  drawText("9 min walk");

  timeClient.update();
  Serial.println(timeClient.getFormattedTime().substring(0,5));

  drawText("Time now:");
  drawText(timeClient.getFormattedTime().substring(0,5));
  // Wait for some time before making the next request
  delay(1000 * 10); // 1 minute
}
