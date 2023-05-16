#include <TinyGPSPlus.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SH110X.h>

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels
#define i2c_Address 0x3c
#define OLED_RESET     -1 // Reset pin # (or -1 if sharing Arduino reset pin)
Adafruit_SH1106G display = Adafruit_SH1106G(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);


// The TinyGPSPlus object


TinyGPSPlus gps;


void setup() {
  Serial.begin(9600);
  Serial2.begin(9600);
  delay(3000);
  display.begin(i2c_Address, true); // Address 0x3C default
  delay(2000);
  display.display();
  display.clearDisplay();
  display.display(); 
}


void loop() {
  //updateSerial();
  while (Serial2.available() > 0)
    if (gps.encode(Serial2.read()))
      displayInfo();

  if (millis() > 5000 && gps.charsProcessed() < 10) {
    Serial.println(F("No GPS detected: check wiring."));
    while (true);
  }
}


void displayInfo() {
  Serial.print(F("Location: "));
  display.clearDisplay();
  display.setTextColor(SH110X_WHITE);
  if (gps.location.isValid()){
    Serial.print("Lat: ");
    Serial.print(gps.location.lat(), 6);
    Serial.print(F(","));
    Serial.print("Lng: ");
    Serial.print(gps.location.lng(), 6);
    Serial.println();
  }

  if (gps.altitude.isValid()) {
    display.setCursor(0,50);
    display.setTextSize(2);
    display.println("Alt");
    display.setCursor(60,50);
    display.println(String(gps.altitude.meters()));
    Serial.println(gps.altitude.meters());}

  if (gps.speed.isValid()) {
    display.setCursor(0,0);
    display.setTextSize(3);
    display.println(String(gps.speed.kmph()));
    display.setCursor(80,0);
    display.setTextSize(1);
    display.println("Alt");    
    Serial.println(gps.speed.kmph());
    
  }
  display.display();    
  if (gps.satellites.isValid()) {
    Serial.println(gps.satellites.value());
  }

  else {
    Serial.print(F("INVALID"));
  }
  
  delay(1000);
}


void updateSerial() {
  delay(500);
  while (Serial.available()) {
    Serial2.write(Serial.read()); //Forward what Serial received to Software Serial Port
  }
  while (Serial2.available()) {
    Serial.write(Serial2.read()); //Forward what Software Serial received to Serial Port
  }
}

