// Reads a button attached to a MCP23XXX pin.

// ok to include only the one needed
// both included here to make things simple for example
#include <Adafruit_MCP23X08.h>
#include <Adafruit_MCP23X17.h>

#define BUTTON_PIN_1 0  // MCP23XXX pin button is attached to
#define BUTTON_PIN_2 1  // MCP23XXX pin button is attached to
#define BUTTON_PIN_3 2  // MCP23XXX pin button is attached to
#define BUTTON_PIN_4 3  // MCP23XXX pin button is attached to
#define BUTTON_PIN_5 4  // MCP23XXX pin button is attached to
#define BUTTON_PIN_6 5  // MCP23XXX pin button is attached to
int count = 0; 

Adafruit_MCP23X17 mcp;

void setup() {
  Serial.begin(9600);
  //while (!Serial);
  Serial.println("MCP23xxx Button Test!");

  // uncomment appropriate mcp.begin
  if (!mcp.begin_I2C()) {
  //if (!mcp.begin_SPI(CS_PIN)) {
    Serial.println("Error.");
    while (1);
  }

  // configure pin for input with pull up
  mcp.pinMode(BUTTON_PIN_1, INPUT_PULLUP);
  mcp.pinMode(BUTTON_PIN_2, INPUT_PULLUP);
  mcp.pinMode(BUTTON_PIN_3, INPUT_PULLUP);
  mcp.pinMode(BUTTON_PIN_4, INPUT_PULLUP);
  mcp.pinMode(BUTTON_PIN_5, INPUT_PULLUP);
  mcp.pinMode(BUTTON_PIN_6, INPUT_PULLUP);

  Serial.println("Looping...");
}

void loop() {
  // LOW = pressed, HIGH = not pressed
  int b1 = mcp.digitalRead(BUTTON_PIN_1); 
  int b2 = mcp.digitalRead(BUTTON_PIN_2); 
  int b3 = mcp.digitalRead(BUTTON_PIN_3); 
  int b4 = mcp.digitalRead(BUTTON_PIN_4); 
  int b5 = mcp.digitalRead(BUTTON_PIN_5); 
  int b6 = mcp.digitalRead(BUTTON_PIN_6); 

 // Serial.println(mcp.digitalRead(BUTTON_PIN_1)); 

  if (b1) {
    // Serial.println("Button 1 Pressed!");
    count += 1; 
  } else if (b2)  {
    // Serial.println("Button 2 Pressed!");
    count += 1; 
  }else if (b3)  {
    // Serial.println("Button 3 Pressed!");
    count += 1; 
  }else if (b4)  {
    // Serial.println("Button 4 Pressed!");
    count += 1; 
  }else if (b5)  {
    // Serial.println("Button 5 Pressed!");
    count += 1; 
  }else if (b6)  {
    // Serial.println("Button 6 Pressed!");
    count +=1 ; 
  } 
    
  Serial.println(count);
  delay(1000); 
}