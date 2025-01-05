#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include "Adafruit_LEDBackpack.h"

#define RESET_PIN A2
#define SENSOR_START_PIN A5
#define SENSOR_END_PIN A4
#define DEBOUNCE_TIME 50 // 50ms debounce

Adafruit_7segment matrix = Adafruit_7segment();

// Timer state variables
uint32_t race_start_time = 0;
uint32_t race_end_time = 0;
uint32_t race_duration = 0;
bool race_in_progress = false;

// Debounce variables
uint32_t last_reset_time = 0;
uint32_t last_start_time = 0;
uint32_t last_end_time = 0;
bool last_reset_state = HIGH;
bool last_start_state = HIGH;
bool last_end_state = HIGH;

void show_time_ms(uint32_t time_ms)
{
  uint32_t seconds = time_ms / 1000;

  matrix.writeDigitNum(0, (seconds % 100) / 10);
  matrix.writeDigitNum(1, seconds % 10);
  matrix.writeDigitNum(3, (time_ms % 1000) / 100);
  matrix.writeDigitNum(4, (time_ms % 100) / 10);

  matrix.drawColon(true);
  matrix.writeDisplay();
}

void setup()
{
  Serial.begin(115200);
  Serial.println("Race Timer Ready");

  matrix.begin(0x70);
  matrix.setBrightness(15);

  pinMode(RESET_PIN, INPUT_PULLUP);
  pinMode(SENSOR_START_PIN, INPUT_PULLUP);
  pinMode(SENSOR_END_PIN, INPUT_PULLUP);
  digitalWrite(SENSOR_START_PIN, HIGH);
}

void loop()
{
  uint32_t current_time = millis();

  // Read all inputs
  bool reset_state = digitalRead(RESET_PIN);
  bool start_state = digitalRead(SENSOR_START_PIN);
  bool end_state = digitalRead(SENSOR_END_PIN);

  // Handle reset button with debounce
  if (reset_state != last_reset_state && (current_time - last_reset_time) > DEBOUNCE_TIME)
  {
    if (reset_state == LOW)
    { // Button pressed
      race_in_progress = false;
      race_end_time = 0;
      race_start_time = 0;
      race_duration = 0;
    }
    last_reset_time = current_time;
    last_reset_state = reset_state;
  }

  // Handle start sensor with debounce
  if (start_state != last_start_state && (current_time - last_start_time) > DEBOUNCE_TIME)
  {
    if (start_state == LOW)
    { // Sensor triggered
      race_start_time = current_time;
      race_end_time = 0;
      race_duration = 0;
      race_in_progress = true;
    }
    last_start_time = current_time;
    last_start_state = start_state;
  }

  // Handle end sensor with debounce
  if (end_state != last_end_state && (current_time - last_end_time) > DEBOUNCE_TIME)
  {
    if (end_state == LOW && race_in_progress)
    { // Sensor triggered
      race_end_time = current_time;
      race_duration = race_end_time - race_start_time;
      race_in_progress = false;
    }
    last_end_time = current_time;
    last_end_state = end_state;
  }

  // Update display
  if (race_in_progress)
  {
    show_time_ms(current_time - race_start_time);
  }
  else if (race_end_time > 0)
  {
    show_time_ms(race_end_time - race_start_time);
  }
  else
  {
    show_time_ms(0);
  }
}