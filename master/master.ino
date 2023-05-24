#include <esp_now.h>
#include <WiFi.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SH110X.h>
#include <TinyGPSPlus.h>

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels
#define i2c_Address 0x3c
#define OLED_RESET -1 // Reset pin # (or -1 if sharing Arduino reset pin)
Adafruit_SH1106G display = Adafruit_SH1106G(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

#define BUTTON_PIN_SCREEN 36
#define BUTTON_PIN_CAL 39

TinyGPSPlus gps;

typedef struct struct_message
{
  int bpm;
} struct_message;

typedef struct struct_data
{
  int bpm;
  double speed;
  double loc_lat;
  double loc_lng;
  double distance;
  double alt;
  int weight;
  double calories;
  int bpmProm;
  int bpmWrites;
} struct_data;

struct_data data = {0, 0.0, 0.0, 0.0, 0.0, 0.0, 50, 0.0, 0, 0};
struct_message msgData;
int displayState = 0;

double speeds[] = {0.0, 13.0, 16.0, 19.0, 22.5, 24.0, 25.5, 27.0, 29.0, 30.55, 32.0, 33.5, 37.0, 40.0};
double coefficient[] = {0.0, 0.000049167, 0.000059167, 0.000071, 0.000085333, 0.0000935, 0.0001025, 0.0001125, 0.000123333, 0.000135167, 0.0001485, 0.0001625, 0.0001955, 0.000235167};

const unsigned char Heart_Icon[] PROGMEM = {
    0x00, 0x00, 0x18, 0x30, 0x3c, 0x78, 0x7e, 0xfc, 0xff, 0xfe, 0xff, 0xfe, 0xee, 0xee, 0xd5, 0x56,
    0x7b, 0xbc, 0x3f, 0xf8, 0x1f, 0xf0, 0x0f, 0xe0, 0x07, 0xc0, 0x03, 0x80, 0x01, 0x00, 0x00, 0x00};

// callback function that will be executed when data is received
void OnDataRecv(const uint8_t *mac, const uint8_t *incomingData, int len)
{
  memcpy(&msgData, incomingData, sizeof(msgData));
  Serial.print("Bytes received: ");
  Serial.println(len);
  Serial.print("BPM: ");
  Serial.println(String(msgData.bpm));

  data.bpmWrites++;
  data.bpmProm += msgData.bpm;
}

void setup()
{
  Serial.begin(115200);
  Serial2.begin(9600);

  // Set device as a Wi-Fi Station
  WiFi.mode(WIFI_STA);

  // Init ESP-NOW
  if (esp_now_init() != ESP_OK)
  {
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

  pinMode(BUTTON_PIN_SCREEN, INPUT);
  pinMode(BUTTON_PIN_CAL, INPUT);
}

void loop()
{
  // updateSerial();
  while (Serial2.available() > 0)
  {
    if (gps.encode(Serial2.read()))
      updateData();
  }

  if (millis() > 5000 && gps.charsProcessed() < 10)
  {
    Serial.println(F("No GPS detected: check wiring."));
    while (true)
      ;
  }
}

void updateData()
{
  checkButtons();

  if (data.bpmWrites == 5)
    {
      data.bpm = data.bpmProm / data.bpmWrites;
      data.bpmProm = 0;
      data.bpmWrites = 0;
    }

  if (gps.satellites.value() >= 5)
  {
    data.speed = gps.speed.kmph();
    data.distance += data.speed * (0.1 / 3600);
    data.loc_lat = gps.location.lat();
    data.loc_lng = gps.location.lng();
    data.alt = gps.altitude.meters();
    data.calories += coefficient[findClosestIndex(speeds, 0, 13, data.speed)] * data.weight;

    if (displayState == 0)
    {
      displayData0();
    }

    if (displayState == 1)
    {
      displayData1();
    }

    if (displayState == 2)
    {
      displayData2();
    }

    if (displayState == 3)
    {
      displayData3();
    }
  }
  else
  {
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

void displayData0()
{
  display.clearDisplay();
  display.setTextColor(SH110X_WHITE);

  display.setCursor(0, 0);
  display.setTextSize(2);
  display.print(String(data.speed));
  display.setTextSize(1);
  display.print("km/h");

  display.setCursor(0, 22);
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

void displayData1()
{
  display.clearDisplay();
  display.setCursor(10, 16);
  display.setTextSize(3);
  display.print(String(data.speed));
  display.setTextSize(1);
  display.print("km/h");
  display.display();
}

void displayData2()
{
  display.clearDisplay();
  display.setCursor(10, 16);
  display.setTextSize(3);
  display.print(String(int(data.calories)));
  display.setTextSize(1);
  display.print("cal");
  display.display();
}

void displayData3()
{
  display.clearDisplay();
  display.setCursor(10, 16);
  display.setTextSize(3);
  display.print(gps.time.hour() - 3);
  display.print(":");
  display.print(gps.time.minute());
  display.display();
}

void updateSerial()
{
  delay(500);
  while (Serial.available())
  {
    Serial2.write(Serial.read()); // Forward what Serial received to Software Serial Port
  }
  while (Serial2.available())
  {
    Serial.write(Serial2.read()); // Forward what Software Serial received to Serial Port
  }
}

void setCalories()
{
  int timer = 0;
  while (timer < 3000)
  {
    Serial.println(data.weight);
    weightSetup();
    delay(200);
    timer += 200;
    if (digitalRead(BUTTON_PIN_SCREEN) == HIGH)
    {
      data.weight++;
      timer = 0;
    }
    else if (digitalRead(BUTTON_PIN_CAL) == HIGH)
    {
      data.weight--;
      timer = 0;
    }
  }
}

void weightSetup()
{
  display.clearDisplay();
  display.setTextColor(SH110X_WHITE);
  display.setCursor(4, 4);
  display.setTextSize(1);
  display.print("Introducir peso:");
  display.setCursor(40, 20);
  display.setTextSize(3);
  display.print(data.weight);
  display.setTextSize(2);
  display.print("kg");
  display.display();
}

void checkButtons()
{
  if (digitalRead(BUTTON_PIN_SCREEN) == HIGH)
  {
    displayState = (displayState + 1) % 4;
    delay(300);
  }
  if (digitalRead(BUTTON_PIN_CAL) == HIGH)
  {
    setCalories();
    delay(300);
  }
}

int findClosestIndex(double arr[], int left, int right, int target)
{
  // base case: when there is only one element in the array
  if (left == right)
  {
    return left;
  }

  // calculate the middle index
  int mid = (left + right) / 2;

  // recursively search the left half of the array
  int leftClosest = findClosestIndex(arr, left, mid, target);

  // recursively search the right half of the array
  int rightClosest = findClosestIndex(arr, mid + 1, right, target);

  // compare the absolute differences of the closest elements in the left and right halves
  if (abs(leftClosest - target) <= abs(rightClosest - target))
  {
    return leftClosest;
  }
  else
  {
    return rightClosest;
  }
}