#include <FastLED.h>

// USB Constants
#define SERIAL_BAUD 9600

// LED Constants
#define DATA_PIN    3
#define CLK_PIN   4
#define LED_TYPE    APA102
#define COLOR_ORDER BGR
#define NUM_LEDS    15
#define BRIGHTNESS          255
#define FRAMES_PER_SECOND  120
CRGB leds[NUM_LEDS];

// Sensor Constants
#define SENSOR_PIN 2

// Barrel Constants
#define BARREL_PIN 13

// DEPRECATED
const int max_brightness = 1;

const int delayval = 100;
int lock_state = 1; // 0 = Unlock, 1 = Auto, 2 = Locked
int status_state = 0; // 0 = Offline, 1 = Available, 2 = Away, 3 = Busy
int last_known_sensor_state = LOW;
int number_of_activations_since_last_tx = 0;

void setup() {
  // Init USB
  Serial.begin(SERIAL_BAUD);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }

  // Init LEDs
  FastLED.addLeds<LED_TYPE,DATA_PIN,CLK_PIN,COLOR_ORDER>(leds, NUM_LEDS).setCorrection(TypicalLEDStrip);
  FastLED.setBrightness(BRIGHTNESS);

  // Init Sensor
  pinMode(SENSOR_PIN, INPUT);

  // Setup Complete
  sendReadyMessage();
}

void loop() {
  checkForRecievedMessages();
  checkToActivateBarrel();
}

void checkForRecievedMessages() {
  // Check buffer
  if (Serial.available() > 0) {
    String command = Serial.readStringUntil('\n');
    if (command == "s available")
    {
      setAvailable();
    }
    else if (command == "s away")
    {
      setAway();
    }
    else if (command == "s busy")
    {
      setBusy();
    }
    else if (command == "s offline")
    {
      setOffline();
    }
    else if (command == "l unlock")
    {
      unlockBarrel();
    }
    else if (command == "l auto")
    {
      autoBarrel();
    }
    else if (command == "l lock")
    {
      lockBarrel();
    }
  }
}

void checkToActivateBarrel() {
  if (lock_state == 0 || (lock_state == 1 && status_state == 1))
  {
    int current_sensor_state = digitalRead(SENSOR_PIN);
    if (current_sensor_state == HIGH && last_known_sensor_state == LOW)
    {
      last_known_sensor_state = HIGH;
      
      activateBarrel();
    }
    if (current_sensor_state == LOW && last_known_sensor_state == HIGH)
    {
      last_known_sensor_state = LOW;
    }
  }
  else
  {
    // Reset barrel state when locked
    last_known_sensor_state = LOW;
  }
}

void sendReadyMessage()
{
  Serial.println("r");
}

void sendActivationCountMessage()
{
  Serial.println("a");
}

void activateBarrel() {
  sendActivationCountMessage();
  digitalWrite(BARREL_PIN, HIGH);
  delay(3000);
  digitalWrite(BARREL_PIN, LOW);
  number_of_activations_since_last_tx++;
}

void setAvailable() {
  /*for(int i=0;i<NUMPIXELS;i++){
    // pixels.Color takes RGB values, from 0,0,0 up to 255,255,255
    pixels.setPixelColor(i, pixels.Color(0,max_brightness,0)); // Moderately bright green color.
    pixels.show(); // This sends the updated pixel color to the hardware.
    delay(delayval); // Delay for a period of time (in milliseconds).
  }*/
  for(int currentLed = 0; currentLed < NUM_LEDS; currentLed = currentLed + 1) {
      leds[currentLed] = CRGB::Green;
      FastLED.show();
      delay(delayval);
  }
  status_state = 1;
}

void setAway() {
  for(int currentLed = 0; currentLed < NUM_LEDS; currentLed = currentLed + 1) {
      leds[currentLed] = CRGB::Orange;
      FastLED.show();
      delay(delayval);
  }
  status_state = 2;
}

void setBusy() {
  for(int currentLed = 0; currentLed < NUM_LEDS; currentLed = currentLed + 1) {
      leds[currentLed] = CRGB::Red;
      FastLED.show();
      delay(delayval);
  }
  status_state = 3;
}

void setOffline() {
  for(int currentLed = 0; currentLed < NUM_LEDS; currentLed = currentLed + 1) {
      leds[currentLed] = CRGB::Black;
      FastLED.show();
      delay(delayval);
  }
  status_state = 0;
}

void unlockBarrel() {
  lock_state = 0;
}

void autoBarrel() {
  lock_state = 1;
}

void lockBarrel() {
  lock_state = 2;
}
