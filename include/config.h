#ifndef CONFIG_H
#define CONFIG_H

// --- HARDWARE PIN MAPPING ---
// --- HARDWARE PIN DEFINITIONS ---
#define PIN_RELAY           23  // Output pin for the water pump relay
#define PIN_BUZZER          18  // Output pin for the buzzer alarm
#define PIN_NEOPIXEL        27  // Data pin for the status RGB LED (WS2812)

#define PIN_DHT             4   // Data pin for DHT22 (Temperature & Humidity)
#define PIN_SOIL_MOISTURE   34  // Analog pin (ADC1) for capacitive soil moisture sensor
#define PIN_LDR             35  // Analog pin (ADC1) for photoresistor (Light intensity)

// --- I2C DISPLAY CONFIGURATION ---
#define I2C_SDA             21
#define I2C_SCL             22
#define LCD_ADDR            0x27
#define LCD_COLS            16
#define LCD_ROWS            2

// // --- BUSINESS LOGIC THRESHOLDS ---
// --- SENSOR CONFIGURATION & HYSTERESIS THRESHOLDS ---
#define SOIL_PUMP_ON_THRESHOLD   30  // Soil moisture percentage to trigger watering (< 30%)
#define SOIL_PUMP_OFF_THRESHOLD  50  // Soil moisture percentage to stop watering (> 50%)
#define MAX_PUMP_ON_TIME_MS      120000 // Runaway protection: max 120 seconds continuous pumping

#define TEMP_HIGH_THRESHOLD      35.0 // Thermal warning threshold in Celsius
// --- NETWORK & MQTT INDUSTRIAL CONFIGURATION ---
// NOTE: In a secure deployment pipeline, these macros can be overridden 
// via build flags (-D) in platformio.ini to prevent leaking secrets in Git.
#define WIFI_SSID        "Wokwi-GUEST"
#define WIFI_PASSWORD    ""
// --- CREDENCIALES CLÚSTER REAL (Para cuando subas a la placa física) ---
// #define MQTT_SERVER      "219024e0d99249d9b97689d9c447714d.s1.eu.hivemq.cloud" // Replace with your local broker IP (e.g., "192.168.1.150")
// #define MQTT_PORT        8883
// #define MQTT_USER        "admin" // Leave blank if broker does not require authentication
// #define MQTT_PASSWORD    "admin1234" // Leave blank if broker does not require authentication
// --- CONFIGURACIÓN PARA WOKWI (Activa ahora) ---
#define MQTT_SERVER   "broker.hivemq.com"
#define MQTT_PORT     1883
#define MQTT_USER     "" 
#define MQTT_PASSWORD ""

#define MQTT_CLIENT_ID   "SmartGardenNode_JMRC_9999"
#endif