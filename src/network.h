#ifndef NETWORK_MANAGER_H
#define NETWORK_MANAGER_H
// --- MQTT PACKET SIZE CONFIGURATION ---
// Increase the default buffer size from 256 bytes to 512 bytes.
// This prevents silent packet drops or truncation when serializing larger JSON payloads
// containing multiple sensor readings, device IDs, and system state flags.
#define MQTT_MAX_PACKET_SIZE 512
#include <PubSubClient.h>
#include <Arduino.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include "config.h"
#include <sensors.h>
// --- OPCIÓN 1: PARA WOKWI (Puerto 1883 sin TLS - Activo ahora) ---
#include <WiFiClient.h>
WiFiClient espClient;
// --- OPCIÓN 2: PARA HARDWARE REAL (Puerto 8883 con TLS - Comentado para Wokwi) ---
// #include <WiFiClientSecure.h>
// WiFiClientSecure espClient

// Instancias de cliente MQTT
PubSubClient mqttClient(espClient);

// Variables de control de tiempo para reconexiones no bloqueantes
unsigned long lastReconnectAttempt = 0;
const unsigned long RECONNECT_INTERVAL_MS = 5000;

void initNetwork() {
    // ========================================================================
    // CONFIGURACIÓN DE SEGURIDAD TLS (Descomenta SOLO para Hardware Real)
    // ========================================================================
    // IMPORTANTE: Omitir la validación estricta del certificado raíz en cloud
    // espClient.setInsecure(); // <- Comentado para Wokwi (el puerto 1883 no usa TLS)

    // Configurar servidor y puerto (lee automáticamente de config.h)
    mqttClient.setServer(MQTT_SERVER, MQTT_PORT);
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
void publishTelemetry(SensorData data, bool pumpState, bool timeoutError,bool isOverheating, bool needsWatering) {
    // 1. Si la conexión se cayó, la reconectamos automáticamente de inmediato
    if (!mqttClient.connected()) {
    Serial.println(F("[MQTT] Connection lost. Attempting reconnection..."));
    if (!reconnectNonBlocking()) {
            Serial.println(F("[MQTT] Reconnection failed. Retrying next cycle."));
            return; // Sale para no bloquear el programa
        }
  }
  // 2. Mantener la pila de red activa obligatoriamente justo antes de serializar
  mqttClient.loop();
    // 2. Estructura del JSON
    StaticJsonDocument<512> doc;
    doc["device_id"] = MQTT_CLIENT_ID;
    doc["uptime_ms"] = millis();
    // Redondeamos a 2 decimales para evitar decimales infinitos como -1.600000024
    doc["temperature"] = roundf(data.temperature * 100.0) / 100.0;
    doc["humidity"] = roundf(data.humidity * 100.0) / 100.0;
    doc["soil_moisture"] = data.soilMoisturePercent;
    doc["light_raw"] = data.lightLevelRaw;
    doc["pump_status"] = pumpState ? "ON" : "OFF";
    doc["safety_lock"] = timeoutError;
    // --- AÑADIMOS LOS ESTADOS Y AVISOS DE FALLO ---
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
    // 3. Publicación del paquete
    if (mqttClient.publish(topic.c_str(), payload)) {
        Serial.println(F("[MQTT] Telemetry packet published successfully."));
    } else {
        Serial.println(F("[ERROR] Failed to publish MQTT telemetry packet."));
        mqttClient.disconnect(); // Forzamos cierre para obligar a reconectar en el siguiente ciclo
    }
}

#endif