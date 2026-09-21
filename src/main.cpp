#include <Arduino.h>
#include "config.h"
#include <sensors.h>  
#include <actuators.h>  
#include <display.h>



// // --- NON-BLOCKING TIMING VARIABLES ---
unsigned long previousMillis = 0;
const long interval = 2000; // Read sensors and update system every 2 seconds

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  Serial.println(F("[INIT] Booting Enterprise Smart Garden Node..."));
  initSensors();
  initActuators();
  initDisplay();
}

void loop() {
unsigned long currentMillis = millis();

    // Non-blocking task execution based on defined interval
    if (currentMillis - previousMillis >= interval) {
        previousMillis = currentMillis;
  
SensorData data = readAllSensors();
// Check each sensor cleanly using the flags processed in sensors.h
if (!data.dhtValid) {
    Serial.println(F("[ERROR] DHT22 sensor failure!"));
}
if (!data.soilValid) {
    Serial.println(F("[ERROR] Soil moisture sensor disconnected or failing!"));
}
if (!data.ldrValid) {
    Serial.println(F("[ERROR] LDR light sensor failure!"));
}
  // 2. Business Logic and Automation Rules
  bool needsWatering = (data.soilMoistureRaw < SOIL_DRY_THRESHOLD);
  bool isOverheating = (data.temperature > TEMP_HIGH_THRESHOLD);
  // Actuate Relay (Watering Pump)
  setWateringPump(needsWatering);

  // Manage Visual States and Audible Alarms
  if (isOverheating) {
    Serial.print(" STATE_WARNING ");
    setSystemVisualState(STATE_WARNING);
    triggerBuzzer(true); // Sound alarm on thermal risk
  } else if (needsWatering) {
    Serial.print(" STATE_WATERING ");
    setSystemVisualState(STATE_WATERING);
    triggerBuzzer(false);
  } else {
    Serial.print(" STATE_NORMAL ");
    setSystemVisualState(STATE_NORMAL);
    triggerBuzzer(false);
  }

  // 3. Serial Telemetry (Industrial Debugging)
  Serial.printf("Temp: %.1f C | Hum: %.1f %% | Soil: %d %%| Light: %d | Pump: %s\n", data.temperature, data.humidity, data.soilMoisturePercent, data.lightLevelRaw, needsWatering ? "ON" : "OFF");
  Serial.println(" ");

  // 4. Update LCD Display with live telemetry
  updateDisplay(data, needsWatering, isOverheating);
}
}