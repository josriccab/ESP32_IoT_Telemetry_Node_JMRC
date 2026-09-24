#include <Arduino.h>
#include "config.h"
#include <sensors.h>  
#include <actuators.h>  
#include <display.h>
#include <network.h>


// --- NON-BLOCKING TIMING VARIABLES ---
unsigned long previousMillis = 0;
const long interval = 2000; // Read sensors and update system every 2 seconds
// Declaras esto fuera del loop (al principio de main.cpp o como variable global)
unsigned long lastTelemetryTime = 0;
const unsigned long telemetryInterval = 5000; // 5000 ms = 5 segundos

// Pump state variable for hysteresis control (prevents relay chatter)
bool pumpState = false;
unsigned long pumpStartTime = 0; // Tracks when the pump started running
bool pumpTimeoutError = false;   // Critical error flag for timeout exceeded

void setup() {
  Serial.begin(9600);
  // Conectar a la red Wi-Fi simulada de Wokwi
  WiFi.begin("Wokwi-GUEST", "");
  
  // Esperar a que se conecte (opcional pero recomendado para depurar)
  while (WiFi.status() != WL_CONNECTED) {
      delay(500);
      Serial.print(".");
  }
  Serial.println(F("\n[WiFi] Connected successfully to Wokwi-GUEST!"));
  // put your setup code here, to run once:
  
  Serial.println(F("[INIT] Booting Enterprise Smart Garden Node..."));
  initSensors();
  initActuators();
  initDisplay();
  initNetwork();
}

void loop() {
  // Mantener el cliente MQTT vivo en segundo plano
  // Es vital que esto se ejecute en cada pasada del loop para mantener la conexión viva
  mqttClient.loop();
  handleNetwork();
// Placed outside the 2-second interval so the operator can clear safety errors immediately.
if (Serial.available() > 0) {
    char command = Serial.read();
    if (command == 'R' || command == 'r') {
        pumpTimeoutError = false;
        pumpStartTime = 0;
        Serial.println(F("[INFO] Pump timeout error manually cleared by operator."));
    }
}

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
if (data.soilValid) {
    if (data.soilMoisturePercent < SOIL_PUMP_ON_THRESHOLD && !pumpTimeoutError) {
      // Hysteresis ON threshold: triggers only when soil drops below SOIL_PUMP_ON_THRESHOLD (e.g., 30%)
      // Also guarded by !pumpTimeoutError to prevent restarting after a safety shutdown  
      if (!pumpState) {
           // Transition: Pump just turned on, record the start time for the watchdog timer
           pumpStartTime = currentMillis;
           pumpState = true;
        } else {
        // Pump was already running: check elapsed time
        if (currentMillis - pumpStartTime > MAX_PUMP_ON_TIME_MS) {
            pumpState = false;       // Emergency shut-off to prevent motor burnout or flooding
            pumpTimeoutError = true; // Lock further startups until soil recovers or system resets
            Serial.println(F("[CRITICAL] Pump timeout! Auto-shutoff engaged."));
        }
     }
} // Hysteresis OFF threshold: pump remains ON across the dead-band zone (30%-50%) 
  // and only shuts off once moisture exceeds SOIL_PUMP_OFF_THRESHOLD (e.g., 50%)
  else if (data.soilMoisturePercent > SOIL_PUMP_OFF_THRESHOLD) {
  pumpState = false;
  pumpTimeoutError = false; // Reset error flag once soil is properly hydrated
  }
} else {
        pumpState = false; // Fail-safe: shut off if sensor disconnects
  }
  
  bool needsWatering = pumpState;
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

  // 5. Publicar telemetría por MQTT hacia la nube
  // Control de tiempo para publicar en la nube sin sobrecargar
  if (millis() - lastTelemetryTime >= telemetryInterval) {
    lastTelemetryTime = millis(); // Actualizamos el cronómetro
        publishTelemetry(data, needsWatering, pumpTimeoutError, isOverheating, needsWatering);
  }
}
}