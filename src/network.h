#ifndef NETWORK_MANAGER_H
#define NETWORK_MANAGER_H

#include <Arduino.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include "config.h"
#include <sensors.h>
// Instancias de cliente de red
WiFiClientSecure espClient;
PubSubClient mqttClient(espClient);

// Variables de control de tiempo para reconexiones no bloqueantes
unsigned long lastReconnectAttempt = 0;
const unsigned long RECONNECT_INTERVAL_MS = 5000;

void initNetwork() {
    // Configurar servidor y puerto estándar (1883)
    mqttClient.setServer(MQTT_SERVER, MQTT_PORT);
    
    // IMPORTANTE: Omitir la validación estricta del certificado raíz para desarrollo ágil en cloud
    //espClient.setInsecure();
    
    // Configurar servidor y puerto seguro de HiveMQ Cloud desde config.h
    //mqttClient.setServer(MQTT_SERVER, MQTT_PORT);
}

// Intento de reconexión no bloqueante (Evita congelar el ESP32)
boolean reconnectNonBlocking() {
    if (mqttClient.connect(MQTT_CLIENT_ID, MQTT_USER, MQTT_PASSWORD)) {
        Serial.println(F("[MQTT] Connected successfully to HiveMQ Cloud via TLS."));
        // Suscripción a tópicos de control remoto si se requiere
        // mqttClient.subscribe("garden/node_01/cmd");
    } else {
        Serial.print(F("[MQTT] Failed, rc="));
        Serial.print(mqttClient.state());
    }
    return mqttClient.connected();
}

void handleNetwork() {
    // 1. Verificar estado de Wi-Fi
    if (WiFi.status() != WL_CONNECTED) {
        return; 
    }

    // 2. Gestionar conexión MQTT sin bloquear el loop principal
    if (!mqttClient.connected()) {
        unsigned long currentMillis = millis();
        if (currentMillis - lastReconnectAttempt >= RECONNECT_INTERVAL_MS) {
            lastReconnectAttempt = currentMillis;
            if (reconnectNonBlocking()) {
                lastReconnectAttempt = 0;
            }
        }
    } else {
        // Mantener vivo el cliente MQTT
        mqttClient.loop();
    }
}

// Publicación de telemetría optimizada y segura con JSON estático
void publishTelemetry(SensorData data, bool pumpState, bool timeoutError) {
    if (!mqttClient.connected()) return;

    StaticJsonDocument<300> doc;
    doc["device_id"] = MQTT_CLIENT_ID;
    doc["uptime_ms"] = millis();
    doc["temperature"] = data.temperature;
    doc["humidity"] = data.humidity;
    doc["soil_moisture"] = data.soilMoisturePercent;
    doc["light_raw"] = data.lightLevelRaw;
    doc["pump_status"] = pumpState ? "ON" : "OFF";
    doc["safety_lock"] = timeoutError;

    char payload[300];
    serializeJson(doc, payload);

    String topic = String("garden/") + MQTT_CLIENT_ID + "/telemetry";
    
    if (mqttClient.publish(topic.c_str(), payload)) {
        Serial.println(F("[MQTT] Telemetry packet published successfully."));
    } else {
        Serial.println(F("[ERROR] Failed to publish MQTT telemetry packet."));
    }
}

#endif