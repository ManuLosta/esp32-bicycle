#include <PulseSensorPlayground.h>

hw_timer_t * sampleTimer = NULL;
portMUX_TYPE sampleTimerMux = portMUX_INITIALIZER_UNLOCKED;

PulseSensorPlayground pulseSensor;

const int PULSE_INPUT = 36;
const int PULSE_BLINK = 13;    
const int PULSE_FADE = 5;
const int THRESHOLD = 685; 

void IRAM_ATTR onSampleTime() {
  portENTER_CRITICAL_ISR(&sampleTimerMux);
  PulseSensorPlayground::OurThis->onSampleTime();
  portEXIT_CRITICAL_ISR(&sampleTimerMux);
}

boolean sendPulseSignal = false;

void setup() {
  Serial.begin(115200);

  analogReadResolution(10);

  pulseSensor.analogInput(PULSE_INPUT);
  pulseSensor.blinkOnPulse(PULSE_BLINK);
  pulseSensor.fadeOnPulse(PULSE_FADE);
  pulseSensor.setSerial(Serial);
  pulseSensor.setThreshold(THRESHOLD);

  sampleTimer = timerBegin(0, 80, true);                
  timerAttachInterrupt(sampleTimer, &onSampleTime, true);  
  timerAlarmWrite(sampleTimer, 2000, true);      
  timerAlarmEnable(sampleTimer);

}

void loop() {
  int myBPM = pulseSensor.getBeatsPerMinute();  

  if (pulseSensor.sawStartOfBeat()) {
   Serial.println(String("BPM: ") + myBPM);                        
  }
  delay(500);  
}

