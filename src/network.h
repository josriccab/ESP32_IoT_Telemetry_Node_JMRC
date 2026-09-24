/**
 * @file network.h
 * @brief Network management, Wi-Fi initialization, and non-blocking MQTT telemetry publisher.
 * @author JMRC
 */

#ifndef NETWORK_MANAGER_H
#define NETWORK_MANAGER_H


#include <Arduino.h>
#include <WiFi.h>
#include <WiFiClient.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include "config.h"
#include <sensors.h>

// --- MQTT BUFFER CONFIGURATION ---
// Increase default buffer size to 512 bytes to prevent silent truncation 
// of JSON payloads containing comprehensive telemetry metrics and state flags.

#define MQTT_MAX_PACKET_SIZE 512


// Network and MQTT client instances
WiFiClient espClient;
PubSubClient mqttClient(espClient);

// Non-blocking reconnection state variables
unsigned long lastReconnectAttempt = 0;
const unsigned long RECONNECT_INTERVAL_MS = 5000;

/**
 * @brief Initializes the MQTT broker settings.
 */

void initNetwork() {
    mqttClient.setServer(MQTT_SERVER, MQTT_PORT);
}

/**
 * @brief Attempts a non-blocking reconnection to the MQTT broker.
 * @return boolean True if connection is successful, false otherwise.
 */

boolean reconnectNonBlocking() {
    if (mqttClient.connect(MQTT_CLIENT_ID, MQTT_USER, MQTT_PASSWORD)) {
        Serial.println(F("[MQTT] Connected successfully to HiveMQ broker."));
    } else {
        Serial.print(F("[MQTT] Failed, rc="));
        Serial.print(mqttClient.state());
    }
    return mqttClient.connected();
}

/**
 * @brief Manages network health, handling Wi-Fi checks and non-blocking MQTT reconnections.
 */

void handleNetwork() {
    // 1. Abort if Wi-Fi link is down
    if (WiFi.status() != WL_CONNECTED) {
        return; 
    }

    // 2. Manage MQTT connection state without blocking the main event loop
    if (!mqttClient.connected()) {
        unsigned long currentMillis = millis();
        if (currentMillis - lastReconnectAttempt >= RECONNECT_INTERVAL_MS) {
            lastReconnectAttempt = currentMillis;
            if (reconnectNonBlocking()) {
                lastReconnectAttempt = 0;
            }
        }
    } else {
        // Process incoming packets and maintain the keep-alive heartbeat
        mqttClient.loop();
    }
}

/**
 * @brief Serializes sensor data and system status into a JSON payload and publishes via MQTT.
 * @param data Struct containing current environmental readings.
 * @param pumpState Current operational state of the water pump.
 * @param timeoutError Flag indicating a safety runaway timeout error.
 * @param isOverheating Flag indicating thermal warning state.
 * @param needsWatering Flag indicating active irrigation state.
 */

void publishTelemetry(SensorData data, bool pumpState, bool timeoutError,bool isOverheating, bool needsWatering) {
    // 1. Ensure broker connectivity before serializing payload
    if (!mqttClient.connected()) {
    Serial.println(F("[MQTT] Connection lost. Attempting reconnection..."));
    if (!reconnectNonBlocking()) {
            Serial.println(F("[MQTT] Reconnection failed. Retrying next cycle."));
            return; // Sale para no bloquear el programa
        }
    }
    // Maintain stack activity prior to serialization
    mqttClient.loop();

    // 2. Construct optimized JSON document using StaticJsonDocument

    StaticJsonDocument<512> doc;
    doc["device_id"] = MQTT_CLIENT_ID;
    doc["uptime_ms"] = millis();
    doc["temperature"] = roundf(data.temperature * 100.0) / 100.0;
    doc["humidity"] = roundf(data.humidity * 100.0) / 100.0;
    doc["soil_moisture"] = data.soilMoisturePercent;
    doc["light_raw"] = data.lightLevelRaw;
    doc["pump_status"] = pumpState ? "ON" : "OFF";
    doc["safety_lock"] = timeoutError;

    // Evaluate system operational states
    if (isOverheating) {
    doc["system_state"] = "WARNING_OVERHEATING";
    } else if (needsWatering) {
    doc["system_state"] = "STATE_WATERING";
    } else {
    doc["system_state"] = "STATE_NORMAL";
    }
    char payload[512];
    serializeJson(doc, payload);

    String topic = String("garden/") + MQTT_CLIENT_ID + "/telemetry";

    // 3. Publish payload to broker
    if (mqttClient.publish(topic.c_str(), payload)) {
        Serial.println(F("[MQTT] Telemetry packet published successfully."));
    } else {
        Serial.println(F("[ERROR] Failed to publish MQTT telemetry packet."));
        mqttClient.disconnect(); // Force clean disconnect to trigger recovery on next cycle
    }
}

#endif