#include <Arduino.h>
#include <U8g2lib.h>
#include <SPI.h>
#include <Wire.h>

#include <ArduinoJson.h>
#include <SoftwareSerial.h>

SoftwareSerial mySerial(6, 7); // RX, TX

U8G2_SH1107_PIMORONI_128X128_1_4W_HW_SPI u8g2(U8G2_R3, /* cs=*/2, /* dc=*/3, /* reset=*/4);

void setup() {
  mySerial.begin(9600);
  Serial.begin(57600);
  
  // Initialize SPI pins
  u8x8_t *u8x8 = u8g2.getU8x8();
  u8x8->pins[U8X8_PIN_SPI_CLOCK] = SCK;
  u8x8->pins[U8X8_PIN_SPI_DATA] = MOSI;
  
  Wire.begin();
  u8g2.begin();
  
  Serial.println("Testing display alignment...");
  testDisplayAlignment();
}

void loop() {
  JsonDocument doc;


  String dataShow = "old";

  int x = 0;
  int y = 0;

  if (mySerial.available()){
    String data = mySerial.readStringUntil('\n');
    
    DeserializationError error = deserializeJson(doc, data);  

    if (error == DeserializationError::Ok && doc.size() > 0) {
      JsonObject firstElement = doc[0];
      if (firstElement.containsKey("center")) {
        JsonArray centerArray = firstElement["center"];
        if (centerArray.size() >= 2) {
          x = centerArray[0];
          y = centerArray[1];
          dataShow = "X:" + String(x) + " Y:" + String(y);
        }
      }
    }
    Serial.println("Ricevuto: " + dataShow);
  }
  
  u8g2.firstPage();
  do {    
    
    u8g2.setFont(u8g2_font_6x10_tr);
    u8g2.drawStr(5, 120, dataShow.c_str());

    u8g2.drawBox(30 +(x/128), 30 +(y/128), 20, 20);
    
  } while (u8g2.nextPage());
  
  delay(500);
}

// Test function to check display alignment
void testDisplayAlignment() {
  u8g2.firstPage();
  do {
    // Draw reference grid
    u8g2.drawFrame(0, 0, 128, 128);        // Outer border
    u8g2.drawFrame(10, 10, 108, 108);      // Inner border
    
    // Corner markers
    u8g2.drawBox(0, 0, 5, 5);              // Top-left
    u8g2.drawBox(123, 0, 5, 5);            // Top-right  
    u8g2.drawBox(0, 123, 5, 5);            // Bottom-left
    u8g2.drawBox(123, 123, 5, 5);          // Bottom-right
    
    // Center cross
    u8g2.drawHLine(58, 64, 12);            // Horizontal line
    u8g2.drawVLine(64, 58, 12);            // Vertical line
    
    // Text positioning test
    u8g2.setFont(u8g2_font_6x10_tr);
    u8g2.drawStr(20, 20, "TOP");
    u8g2.drawStr(45, 64, "CENTER");
    u8g2.drawStr(20, 115, "BOTTOM");
    
    // Numbers to check wrap-around
    u8g2.setFont(u8g2_font_4x6_tr);
    for(int i = 0; i < 10; i++) {
      char num[3];
      sprintf(num, "%d", i);
      u8g2.drawStr(20 + i * 10, 40, num);
    }
    
  } while (u8g2.nextPage());
  
  delay(3000);
}