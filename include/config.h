/**
 * @file config.h
 * @brief System configuration, hardware mapping, and network parameters for ESP32 IoT Smart Garden.
 * @author JMRC
 */

#ifndef CONFIG_H
#define CONFIG_H

// ==========================================
// HARDWARE PIN MAPPING
// ==========================================

#define PIN_RELAY           23  // Output pin for the water pump relay
#define PIN_BUZZER          18  // Output pin for the buzzer alarm
#define PIN_NEOPIXEL        27  // Data pin for the status RGB LED (WS2812)

#define PIN_DHT             4   // Data pin for DHT22 (Temperature & Humidity sensor)
#define PIN_SOIL_MOISTURE   34  // Analog pin (ADC1) for capacitive soil moisture sensor
#define PIN_LDR             35  // Analog pin (ADC1) for photoresistor (Light intensity)

// ==========================================
// I2C DISPLAY CONFIGURATION
// ==========================================

#define I2C_SDA             21
#define I2C_SCL             22
#define LCD_ADDR            0x27
#define LCD_COLS            16
#define LCD_ROWS            2

// ==========================================
// BUSINESS LOGIC & HYSTERESIS THRESHOLDS
// ==========================================

#define SOIL_PUMP_ON_THRESHOLD   30  // Soil moisture percentage to trigger watering (< 30%)
#define SOIL_PUMP_OFF_THRESHOLD  50  // Soil moisture percentage to stop watering (> 50%)
#define MAX_PUMP_ON_TIME_MS      120000 // Runaway protection: max 120 seconds continuous pumping

#define TEMP_HIGH_THRESHOLD      35.0 // Thermal warning threshold in Celsius

// ==========================================
// NETWORK & MQTT INDUSTRIAL CONFIGURATION
// ==========================================

#define WIFI_SSID        "Wokwi-GUEST"
#define WIFI_PASSWORD    ""

// Active broker configuration (Wokwi simulation profile)
#define MQTT_SERVER   "broker.hivemq.com"
#define MQTT_PORT     1883
#define MQTT_USER     "" 
#define MQTT_PASSWORD ""

#define MQTT_CLIENT_ID   "SmartGardenNode_JMRC_9999"

#endif // CONFIG_H