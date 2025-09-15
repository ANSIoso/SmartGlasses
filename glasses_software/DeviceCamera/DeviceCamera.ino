#include "esp_camera.h"
#include "CameraConf.h"
#include <WiFi.h>
#include <HTTPClient.h>

const char* ssid = "AndroidOp";
const char* password = "certosino";
const char* serverURL = "http://192.168.43.5:5000/upload";  // Replace with your PC's IP

// baud rate info
# define BR_COMUNICATION 9600 // to talk with ArduinoNano 
# define BR_DEBUG 115200      // to check device outputs

camera_fb_t * fb = NULL;

void setup() {
  Serial1.begin(BR_COMUNICATION, SERIAL_8N1, 1, 3);
  Serial.begin(BR_DEBUG);

  // definizione parametri configurazione camera  
  camera_config_t config = createCameraConf();

  // inizializzazione della camera con la configurazione definita
  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("Camera init failed with error 0x%x", err);
    return;
  }

  // connessione al wifi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial1.println("connect");
    Serial.print(".");
  }
  Serial.println("");

  // se la connessione è andata a buon fine debug
  Serial.println("WiFi connected!");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
}

void captureImage() {
  // catturo frame dalla camera
  fb = esp_camera_fb_get();
  if(!fb) {
    Serial.println("Camera capture failed");
    return;
  }
}
void captureClean() {
  esp_camera_fb_return(fb);
}

void sendImageToPython() {
  // [ ->] === stato ===  
  // preparo il "paccetto" con il frame per l'invio
  HTTPClient http;
  http.begin(serverURL);
  http.addHeader("Content-Type", "image/jpeg");
  
  // [<->] === stato ===
  // invio il "pacchetto" e ottengo la "risposta"
  int httpResponseCode = http.POST(fb->buf, fb->len);
  
  // [<- ] === stato ===
  // controllo se la cominicazione è andata a buon fine

  if(httpResponseCode > 0) {
    String response = http.getString();    
    
    Serial.println("HTTP Response: " + String(httpResponseCode));
    Serial.println("HTTP Response: " + response);
    Serial1.println(response);

  } else {
    Serial1.println("com_err");
    Serial.println("Error sending POST request");
  }
  
  http.end();
}

void loop() {
  captureImage();
  sendImageToPython();
  captureClean();
}