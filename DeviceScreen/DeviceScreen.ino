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



void loop() {
  visualizeObjects();
  
  delay(50);
}

void visualizeObjects(){
  if (!mySerial.available())
    return;
    
  JsonDocument doc;
  String data = mySerial.readStringUntil('\n');
  DeserializationError error = deserializeJson(doc, data);
  if (error != DeserializationError::Ok || doc.size() <= 0)
    return;
  
  const int centerX = SCREEN_WIDTH / 2;   // 64
  const int centerY = SCREEN_HEIGHT / 2;  // 64
  const int radius = 40;  // Radius for arrow placement
  
  u8g2.firstPage();
  do {    

    u8g2.drawCircle(centerX, centerY, radius-2, U8G2_DRAW_ALL);
    u8g2.drawCircle(centerX, centerY, radius-3, U8G2_DRAW_ALL);
    u8g2.drawCircle(centerX, centerY, radius+2, U8G2_DRAW_ALL);
    u8g2.drawCircle(centerX, centerY, radius+3, U8G2_DRAW_ALL);
   
    // Process each object
    for(int i = 0; i < doc.size(); i++) {
      JsonObject obj = doc[i];
     
      // Get object center in image coordinates
      long objX = obj["center"][0];
      long objY = obj["center"][1];
      
      // Convert to screen coordinates relative to center
      int relativeX = ((objX * SCREEN_WIDTH) / IMG_WIDTH) - centerX;
      int relativeY = ((objY * SCREEN_HEIGHT) / IMG_HEIGHT) - centerY;
      
      // Calculate angle using atan2 (returns radians)
      float angle = atan2(relativeY, relativeX);
      
      // Calculate arrow position on circumference
      int arrowX = centerX + (int)(cos(angle) * radius);
      int arrowY = centerY + (int)(sin(angle) * radius);
      
      // Optional: Add distance indication with different arrow sizes
      float distance = sqrt(relativeX*relativeX + relativeY*relativeY);

      // Draw directional arrow
      if(distance >= radius - 5)
        drawDirectionalArrow(arrowX, arrowY, angle, (SCREEN_HEIGHT - distance)/3);
      else{
        long x = (objX * SCREEN_WIDTH) / IMG_WIDTH;
        long y = (objY * SCREEN_HEIGHT) / IMG_HEIGHT;
      
        drawCenterCross(x, y);
      }
    }
   
  } while (u8g2.nextPage());
}

void drawCenterCross(long centerX, long centerY){
  u8g2.drawLine(centerX-5, centerY-1, centerX+5,  centerY-1);
  u8g2.drawLine(centerX-5, centerY,   centerX+5,  centerY);
  u8g2.drawLine(centerX-5, centerY+1, centerX+5,  centerY+1);

  u8g2.drawLine(centerX-1, centerY-5, centerX-1,  centerY+5);
  u8g2.drawLine(centerX,   centerY-5, centerX,    centerY+5);
  u8g2.drawLine(centerX+1, centerY-5, centerX+1,  centerY+5);
}

// Function to draw directional arrow pointing outward
void drawDirectionalArrow(int x, int y, float angle, int arrowSize) {  
  // Calculate arrow tip (pointing in the direction of the object)
  int tipX = x + (int)(cos(angle) * arrowSize);
  int tipY = y + (int)(sin(angle) * arrowSize);
  
  // Calculate arrow base points (perpendicular to direction)
  float perpAngle1 = angle + 2.356; // angle + 135 degrees in radians
  float perpAngle2 = angle - 2.356; // angle - 135 degrees in radians
  
  int baseX1 = x + (int)(cos(perpAngle1) * (arrowSize/2));
  int baseY1 = y + (int)(sin(perpAngle1) * (arrowSize/2));
  
  int baseX2 = x + (int)(cos(perpAngle2) * (arrowSize/2));
  int baseY2 = y + (int)(sin(perpAngle2) * (arrowSize/2));
  
  // Draw the arrow triangle
  u8g2.drawTriangle(tipX, tipY, baseX1, baseY1, baseX2, baseY2);
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