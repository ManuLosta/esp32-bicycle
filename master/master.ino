#include <esp_now.h>
#include <WiFi.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SH110X.h>
#include <TinyGPSPlus.h>

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels
#define i2c_Address 0x3c
#define OLED_RESET     -1 // Reset pin # (or -1 if sharing Arduino reset pin)
Adafruit_SH1106G display = Adafruit_SH1106G(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

#define BUTTON_PIN 36

TinyGPSPlus gps;

typedef struct struct_message {
  int bpm;
} struct_message;

typedef struct struct_data {
  int bpm;
  double speed;
  double loc_lat;
  double loc_lng;
  double distance;
  double alt;
} struct_data;

struct_data data = {0, 0.0, 0.0, 0.0, 0.0, 0.0};
struct_message msgData;
int displayState = 0; 

const unsigned char Heart_Icon [] PROGMEM = {
  0x00, 0x00, 0x18, 0x30, 0x3c, 0x78, 0x7e, 0xfc, 0xff, 0xfe, 0xff, 0xfe, 0xee, 0xee, 0xd5, 0x56, 
  0x7b, 0xbc, 0x3f, 0xf8, 0x1f, 0xf0, 0x0f, 0xe0, 0x07, 0xc0, 0x03, 0x80, 0x01, 0x00, 0x00, 0x00
};

// callback function that will be executed when data is received
void OnDataRecv(const uint8_t * mac, const uint8_t *incomingData, int len) {
  memcpy(&msgData, incomingData, sizeof(msgData));
  Serial.print("Bytes received: ");
  Serial.println(len);
  Serial.print("BPM: ");
  Serial.println(String(msgData.bpm));

  data.bpm = msgData.bpm;
}

void setup() {
  Serial.begin(115200);
  Serial2.begin(9600);
  
  // Set device as a Wi-Fi Station
  WiFi.mode(WIFI_STA);

  // Init ESP-NOW
  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }
  
  // Once ESPNow is successfully Init, we will register for recv CB to
  // get recv packer info
  esp_now_register_recv_cb(OnDataRecv);

  display.begin(i2c_Address, true); // Address 0x3C default
  delay(2000);
  display.display();
  display.clearDisplay();
  display.display();

  pinMode(BUTTON_PIN, INPUT);
}

void loop() {
  if (digitalRead(BUTTON_PIN) == HIGH) {
    displayState = (displayState + 1) % 2;
  }

  //updateSerial();
  while (Serial2.available() > 0) {
    if (digitalRead(BUTTON_PIN) == HIGH) {
      displayState = (displayState + 1) % 2;
    }

    if (gps.encode(Serial2.read()))
      updateData();
  }

  if (millis() > 5000 && gps.charsProcessed() < 10) {
    Serial.println(F("No GPS detected: check wiring."));
    while (true);
  }
}


void updateData() {
  if (digitalRead(BUTTON_PIN) == HIGH) {
    displayState = (displayState + 1) % 2;
  }

  if (gps.satellites.value() >= 5) {
    data.speed = gps.speed.kmph();
    data.distance += data.speed * (0.1 / 3600);
    data.loc_lat = gps.location.lat();
    data.loc_lng = gps.location.lng();
    data.alt = gps.altitude.meters();

    Serial.println(data.speed);
    Serial.println(data.distance);
    Serial.println(data.loc_lng);
    Serial.println(data.loc_lng);

    if (displayState == 0) {
      displayData0();
    }

    if (displayState == 1) {
      displayData1();
    }

  } else {
    display.clearDisplay();
    display.setTextColor(SH110X_WHITE);
    display.setCursor(0, 12);
    display.setTextSize(2);
    display.println("Buscando");
    display.println("Satelites");
    display.display();
  }

  delay(100);
}


void displayData0() {
  display.clearDisplay();
  display.setTextColor(SH110X_WHITE);

  display.setCursor(0,0);
  display.setTextSize(2);
  display.print(String(data.speed));
  display.setTextSize(1);
  display.print("km/h");

  display.setCursor(0,22);
  display.setTextSize(1.5);
  display.print(String(data.distance));
  display.print("km");

  display.setCursor(18, 32);
  display.setTextSize(1.5);
  display.drawBitmap(0, 32, Heart_Icon, 16, 16, SH110X_WHITE);
  display.print(String(data.bpm));
  display.print(" BPM");

  display.display();
}

void displayData1() {
  display.clearDisplay();
  display.setCursor(10,16);
  display.setTextSize(3);
  display.print(String(data.speed));
  display.setTextSize(1);
  display.print("km/h");
  display.display();
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


double calculateDistance(double lng, double lat) {
  return pow(pow(lng, 2) + pow(lat, 2), 1/2);
}