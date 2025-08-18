#include <Arduino.h>
#include <U8g2lib.h>
#include <SPI.h>
#include <Wire.h>

#include <ArduinoJson.h>
#include <SoftwareSerial.h>

SoftwareSerial mySerial(6, 7); // RX, TX

U8G2_SH1107_PIMORONI_128X128_1_4W_HW_SPI u8g2(U8G2_R1, /* cs=*/2, /* dc=*/3, /* reset=*/4);

// baud rate info
# define BR_COMUNICATION 9600 // to talk with esp32
# define BR_DEBUG 57600       // to check device outputs

// image settings
# define IMG_HEIGHT 480
# define IMG_WIDTH 640

// screen settings
# define SCREEN_HEIGHT 128
# define SCREEN_WIDTH 128

void setup() {
  mySerial.begin(BR_COMUNICATION);
  Serial.begin(BR_DEBUG);
  
  // Initialize SPI pins
  u8x8_t *u8x8 = u8g2.getU8x8();
  u8x8->pins[U8X8_PIN_SPI_CLOCK] = SCK;
  u8x8->pins[U8X8_PIN_SPI_DATA] = MOSI;
  
  Wire.begin();
  u8g2.begin();
  
  Serial.println("Testing display alignment...");
  testDisplayAlignment();
}

long x = 0;
long y = 0;

void loop() {
  JsonDocument doc;

  if (mySerial.available()){

    String data = mySerial.readStringUntil('\n');

    DeserializationError error = deserializeJson(doc, data);  

    if (error == DeserializationError::Ok && doc.size() > 0) {
      JsonObject firstElement = doc[0];



      JsonArray centerArray = firstElement["center"];

      x = centerArray[0];
      y = centerArray[1];

      x = (x * SCREEN_WIDTH) / IMG_WIDTH;
      y = (y * SCREEN_HEIGHT) / IMG_HEIGHT;
      //dataShow = "X:" + String(x) + " Y:" + String(y);
      //Serial.println("Ricevuto: " + dataShow);
    }else
      return

    u8g2.firstPage();
    do {    
      
      u8g2.setFont(u8g2_font_6x10_tr);

      Serial.println(String(x) + " " + String(y));
      u8g2.drawBox(x, y, 20, 20);
      
    } while (u8g2.nextPage());
  }
  
  
  delay(600);
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