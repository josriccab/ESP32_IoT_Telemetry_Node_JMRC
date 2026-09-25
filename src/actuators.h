/**
 * @file actuators.h
 * @brief Actuator management, PWM buzzer control, NeoPixel status indicator, and relay pump handling.
 * @author JMRC
 */

#ifndef ACTUATORS_H
#define ACTUATORS_H

#include <Arduino.h>
#include <Adafruit_NeoPixel.h>
#include "config.h"

// --- NEOPIXEL OBJECT INITIALIZATION ---
// Parameters: Number of pixels (1), Pin (PIN_NEOPIXEL), Pixel type (GRB + 800KHz)
Adafruit_NeoPixel strip(1, PIN_NEOPIXEL, NEO_GRB + NEO_KHZ800);

/**
 * @brief Enumeration representing the high-level operational states of the system.
 */
enum SystemState {
	STATE_NORMAL,   // Green: System OK
	STATE_WATERING, // Blue: Active irrigation
	STATE_WARNING   // Red: Thermal or critical alert
};

/**
 * @brief Initializes actuators hardware, configures pins, sets initial safe states, and sets up PWM channels.
 */

void initActuators() {
	pinMode(PIN_RELAY, OUTPUT);
	pinMode(PIN_BUZZER, OUTPUT);

	// Ensure safe default state (everything OFF at startup)
	digitalWrite(PIN_RELAY, LOW);
	digitalWrite(PIN_BUZZER, LOW);

	// Initialize classic LEDC PWM channel for the buzzer (ESP32 Core v2.x compatible)
    // Parameters: channel (0), frequency (2000 Hz), resolution (8 bits)
    ledcSetup(0, 2000, 8);
    ledcAttachPin(PIN_BUZZER, 0);
	
	strip.begin();
	strip.setBrightness(50); // Set moderate brightness to protect power supply and prevent glare
	strip.show();            // Initialize strip to "off" state
}

/**
 * @brief Controls the physical water pump relay.
 * @param state Boolean flag: true to activate pump, false to stop.
 */

void setWateringPump(bool state) {
	digitalWrite(PIN_RELAY, state ? HIGH : LOW);
}

/**
 * @brief Updates the status RGB NeoPixel color based on the system state.
 * @param state SystemState enum value representing current operational mode.
 */

void setSystemVisualState(SystemState state) {
	switch (state) {
		case STATE_NORMAL:
			strip.setPixelColor(0, strip.Color(0, 255, 0)); // Green
			break;
		case STATE_WATERING:
			strip.setPixelColor(0, strip.Color(0, 0, 255)); // Blue
			break;
		case STATE_WARNING:
			strip.setPixelColor(0, strip.Color(255, 0, 0)); // Red
			break;
	}
	strip.show();
}

/**
 * @brief Controls the audible alert buzzer.
 * @param state Boolean flag: true to turn tone on, false to turn off.
 */

void triggerBuzzer(bool state) {
	state ? tone(PIN_BUZZER, 1000) : noTone(PIN_BUZZER);
}

#endif // ACTUATORS_H
