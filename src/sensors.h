#ifndef SENSORS_H
#define SENSORS_H

#include <Arduino.h>
#include <DHT.h>
#include "config.h"

// --- SENSOR OBJECT INITIALIZATION ---
DHT dht(PIN_DHT, DHT22);

// --- DATA STRUCTURE FOR CLEAN DATA TRANSFER ---
struct SensorData {
float temperature;
float humidity;
int soilMoistureRaw; // Raw reading (0 - 4095) for debugging
int soilMoisturePercent; // Mapped value (0 - 100%) for the HMI screen
int lightLevelRaw;
bool dhtValid;   // Validates the DHT22
bool soilValid;  // Validates the soil moisture sensor
bool ldrValid;   // Validates the LDR
};

// --- INITIALIZE SENSORS HARDWARE ---
void initSensors() {
 	dht.begin();
	pinMode(PIN_SOIL_MOISTURE, INPUT);
	pinMode(PIN_LDR, INPUT);
 }

// --- READ ALL FIELD VARIABLES ---
 SensorData readAllSensors() {
 	SensorData data;
	// 1. Read and validate DHT22 (Ambient Temperature & Humidity)
 	if (isnan(data.temperature) || isnan(data.humidity)) {
        data.dhtValid = false;
        data.temperature = 0.0f;
        data.humidity = 0.0f;
    } else {
        data.dhtValid = true;
        data.temperature = dht.readTemperature();
 	    data.humidity = dht.readHumidity();
    }
	// 2. Read and validate Soil Moisture Sensor (Analog ADC)
    data.soilMoistureRaw = analogRead(PIN_SOIL_MOISTURE);
    // Defensive check: Detect physical disconnection (open circuit / rail short)
    if (data.soilMoistureRaw <= 15 || data.soilMoistureRaw >= 4080) {
        data.soilValid = false;
        data.soilMoisturePercent = 0; // Safe default state
    } else {
        data.soilValid = true;
        // Map the raw value (0-4095) to a soil moisture percentage (0-100%) and constrain
        int mappedValue = map(data.soilMoistureRaw, 4095, 0, 100, 0); // Ajustado según calibración típica (seco/húmedo)
        data.soilMoisturePercent = constrain(mappedValue, 0, 100);
    }
	// 3. Read and validate LDR (Light Dependent Resistor)
    data.lightLevelRaw = analogRead(PIN_LDR);
    // Defensive check for LDR disconnection
    if (data.lightLevelRaw <= 5 || data.lightLevelRaw >= 4080) {
        data.ldrValid = false;
        data.lightLevelRaw = 0;
    } else {
        data.ldrValid = true;
    }
 	return data;
 }

 #endif
