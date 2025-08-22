#include <Arduino.h>
#include <U8g2lib.h>
#include <SPI.h>
#include <Wire.h>

#include <ArduinoJson.h>
#include <SoftwareSerial.h>

#include "MenuLogic.h"
#include "MenuView.h"


// ====== COMUNICATION ======
// file stream
SoftwareSerial mySerial(6, 7); // RX, TX
String data;
JsonDocument doc;
// baud rate info
# define BR_COMUNICATION 9600 // to talk with esp32
# define BR_DEBUG 57600       // to check device outputs

// ====== VISUALIZATION ======
U8G2_SH1107_PIMORONI_128X128_1_4W_HW_SPI u8g2(U8G2_R1, /* cs=*/2, /* dc=*/3, /* reset=*/4);
// image settings
# define IMG_HEIGHT 480
# define IMG_WIDTH 640
// screen settings
# define SCREEN_HEIGHT 128
# define SCREEN_WIDTH 128

// ====== NAVIGATION ======
// logic
MenuLogic screenMenuL(MenuBitmaps::ICON_COUNT);
int menu_btn_clicked = 0;
// buttons pins
#define BUTTON_UP 9
#define BUTTON_SELECT 5
#define BUTTON_DOWN 8



void setup() {
  mySerial.begin(BR_COMUNICATION);
  Serial.begin(BR_DEBUG);
  
  // Initialize SPI pins
  u8x8_t *u8x8 = u8g2.getU8x8();
  u8x8->pins[U8X8_PIN_SPI_CLOCK] = SCK;
  u8x8->pins[U8X8_PIN_SPI_DATA] = MOSI;
  
  Wire.begin();
  u8g2.begin();
  
  pinMode(BUTTON_UP, INPUT_PULLUP);
  pinMode(BUTTON_SELECT, INPUT_PULLUP);
  pinMode(BUTTON_DOWN, INPUT_PULLUP);
  Serial.println("Start...");
}

void loop() {

  // ---- handle menù navigation (via buttons) ----
  if((menu_btn_clicked == 0) && (digitalRead(BUTTON_SELECT) == LOW)){
		screenMenuL.toggleMenu();
		menu_btn_clicked = 1;
	}

	if((menu_btn_clicked == 1) && (digitalRead(BUTTON_SELECT) == HIGH)){
		menu_btn_clicked = 0;
	}
	

  if(digitalRead(BUTTON_UP) == LOW)
    screenMenuL.nextItem();
  if(digitalRead(BUTTON_DOWN) == LOW)
    screenMenuL.previousItem();

  // ---- handle comunication with esp ----  
  if(!getObjs())
    return;

  
  // ---- handle screen visualizzation ----
	if(screenMenuL.isMenuActive())
    DrawMenu();
  else
    VisualizeObjects();
}

// ======== OBJ UTILS ========
bool getObjs(){
  // check if there are data available to receive from esp
  if (!mySerial.available())
    return false;

  // if available get the data
  data = mySerial.readStringUntil('\n');

  // if data are not useful return error
  if (data == "")
    return false;

  // convert the data string in a json file to access to single elements
  DeserializationError error = deserializeJson(doc, data);
  if (error != DeserializationError::Ok)
  return false;

  // if all went good return success
  return true;
}

int CountClassObjects(char className[]){
  int objCount = 0;

  // going for all the found objects
  for(int i = 0; i < doc.size(); i++) {
    JsonObject obj = doc[i];
    
    // count all the istances of the specified class
    if(obj["class"] == className)
      objCount++;
  }

  return objCount;
}

// ======== DRAWING METODS ========
void VisualizeObjects(){  
  const int screenCenterX = SCREEN_WIDTH / 2;   
  const int screenCenterY = SCREEN_HEIGHT / 2;  
  const int radarRadius = 40;  // radius for arrow placement

  u8g2.firstPage();
  do {    

    // draw radar contour
    u8g2.drawCircle(screenCenterX, screenCenterY, radarRadius-2, U8G2_DRAW_ALL);
    u8g2.drawCircle(screenCenterX, screenCenterY, radarRadius-3, U8G2_DRAW_ALL);
    u8g2.drawCircle(screenCenterX, screenCenterY, radarRadius+2, U8G2_DRAW_ALL);
    u8g2.drawCircle(screenCenterX, screenCenterY, radarRadius+3, U8G2_DRAW_ALL);
   
    // Process each object
    for(int i = 0; i < doc.size(); i++) {
      JsonObject obj = doc[i];

      // check that the istances are of the specified class      
      if(obj["class"] != MenuBitmaps::object_tipes[screenMenuL.getSelectedItem()])
        continue;
     
      // Get object center in image coordinates
      long objX = obj["center"][0];
      long objY = obj["center"][1];
      
      // Convert to screen coordinates relative to center
      int relativeX = ((objX * SCREEN_WIDTH) / IMG_WIDTH) - screenCenterX;
      int relativeY = ((objY * SCREEN_HEIGHT) / IMG_HEIGHT) - screenCenterY;
      
      // Calculate angle using atan2 (returns radians)
      float angle = atan2(relativeY, relativeX);
      
      // Calculate arrow position on circumference
      int arrowX = screenCenterX + (int)(cos(angle) * radarRadius);
      int arrowY = screenCenterY + (int)(sin(angle) * radarRadius);
      
      // Optional: Add distance indication with different arrow sizes
      float distance = sqrt(relativeX*relativeX + relativeY*relativeY);

      // Based on the distance from the center
      if(distance >= radarRadius - 5){
        // Draw directional arrow
        drawDirectionalArrow(arrowX, arrowY, angle, (SCREEN_HEIGHT - distance)/3);
      }else{
        // Draw the precise location
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

void DrawMenu(){
  int item_selected = screenMenuL.getSelectedItem();
  int item_previous = screenMenuL.getPreviousItem();
  int item_next = screenMenuL.getNextItem();

  // count all the istances of the specified class and prepare visualization string
  int objCount = CountClassObjects(MenuBitmaps::object_tipes[screenMenuL.getSelectedItem()]);
  String foundInfo = "Found " + String(objCount);

  u8g2.firstPage();
  do {    
    u8g2.setBitmapMode(1);  // set drawing mode to "combine" the bitmaps
    
    // === draw the interface elements ===
    // - dashboard (to fit the classes icons)
    u8g2.drawXBMP(40, 33, 48, 48, MenuBitmaps::epd_bitmap_dashboard);

    // - scrollbar (to show the progression in the classes list)
          int scrollbarPos = 5 + ((128/MenuBitmaps::ICON_COUNT) * item_selected);
    u8g2.drawXBMP(0, 6, 128, 8, MenuBitmaps::epd_bitmap_scrollbar);
    u8g2.drawXBMP(scrollbarPos, 6, 24, 8, MenuBitmaps::epd_bitmap_scrollbar_handle);

    // === fill the interface
    // - with the actual selected class
    // - and the previos and next one
    u8g2.drawXBMP(6, 42, 32, 32, MenuBitmaps::icon_bitmaps[item_previous]);
    u8g2.drawXBMP(49, 42, 32, 32, MenuBitmaps::icon_bitmaps[item_selected]);
    u8g2.drawXBMP(92, 42, 32, 32, MenuBitmaps::icon_bitmaps[item_next]);

    // - draw the number of istance for the selected class
    u8g2.setFont(u8g2_font_crox5hb_tf);
    u8g2.drawStr(18, 110, foundInfo.c_str());
    
    u8g2.setBitmapMode(0); // set drawing mode to normal

  } while (u8g2.nextPage());
}