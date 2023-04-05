// include the library code:
#include <LiquidCrystal.h>
#include "RTClib.h"
 
// initialize the library with the numbers of the interface pins
LiquidCrystal lcd(19, 23, 18, 17, 16, 15);
RTC_DS3231 rtc;
const int trigPin = 12;
const int echoPin = 14;

//define sound speed in cm/uS
#define SOUND_SPEED 0.034

long duration;
int distanceCm;
char dias[7][12] = {"Domingo", "Lunes", "Martes", "Miercoles", "Jueves", "Viernes", "Sabado"};
 
void setup() {
  lcd.begin(16, 2);   // set up the LCD's number of columns and rows:
  Serial.begin(115200); // Starts the serial communication
  pinMode(trigPin, OUTPUT); // Sets the trigPin as an Output
  pinMode(echoPin, INPUT); // Sets the echoPin as an Input

  if (! rtc.begin()) {
    Serial.println("Couldn't find RTC");
    while (1);
  }

  rtc.adjust(DateTime(__DATE__, __TIME__));
}
 
void loop() {
  DateTime now = rtc.now();
  
  lcd.clear();
  // Clears the trigPin
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
   // Sets the trigPin on HIGH state for 10 micro seconds
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  // Reads the echoPin, returns the sound wave travel time in microseconds
  duration = pulseIn(echoPin, HIGH);
  
  // Calculate the distance
  distanceCm = duration * SOUND_SPEED/2;
  
  // Prints the distance in the Serial Monitor
  Serial.print("Distance (cm): ");
  Serial.println(distanceCm);
  
  lcd.print("Distancia: ");
  lcd.print(distanceCm);
  lcd.print("cm");
  lcd.setCursor(0, 1);
  lcd.print(dias[now.dayOfTheWeek()]);
  lcd.print(" ");
  lcd.print(now.hour());
  lcd.print(":");
  lcd.print(now.minute());
  lcd.setCursor(0, 0);
  
  delay(500);
}