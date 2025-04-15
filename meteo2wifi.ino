/**
 * This code reads temperature, humidity, pressure, and battery voltage
 * from a BME280 sensor and an analog battery monitor on an ESP32.
 * It connects to Wi-Fi, sends the data as a JSON payload to a specified server,
 * then goes into deep sleep for a set duration to conserve power.
 * 
 * Designed for low-power, periodic environmental data collection.
 */

#include <WiFi.h>
#include <HTTPClient.h>
#include <Wire.h>
#include <Adafruit_BME280.h>
#include <ESP32AnalogRead.h>
#include "esp_sleep.h"

// Replace these with your actual credentials and server URL
#define WIFI_SSID "YOUR_WIFI_SSID"
#define WIFI_PASSWORD "YOUR_WIFI_PASSWORD"
#define SERVER_URL "http://your.server.address/data"

#define ADC_PIN 0                 
#define SDA 19
#define SCL 18
#define PIN_ON 3
#define deviderRatio 1.7693877551 
#define SLEEP_TIME 300000000 // 300 seconds (5 minutes) in microseconds

Adafruit_BME280 bme;
ESP32AnalogRead adc;

void setup() {
    Serial.begin(115200);
    
    pinMode(PIN_ON, OUTPUT);
    digitalWrite(PIN_ON, HIGH);

    Wire.begin(SDA, SCL);

    if (!bme.begin()) {
        Serial.println("BME280 not found");
        while (1) delay(1);
    }

    // Connect to Wi-Fi
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    Serial.print("Connecting to WiFi...");
    while (WiFi.status() != WL_CONNECTED) {
        delay(1000);
        Serial.print(".");
    }
    Serial.println("\nConnected to WiFi");

    sendData();
    
    Serial.println("Going to sleep...");
    esp_sleep_enable_timer_wakeup(SLEEP_TIME); // Set deep sleep duration
    esp_deep_sleep_start(); // Enter deep sleep
}

void sendData() {
    if (WiFi.status() == WL_CONNECTED) {
        HTTPClient http;
        http.begin(SERVER_URL);
        http.addHeader("Content-Type", "application/json");

        // Read sensor data
        adc.attach(ADC_PIN);
        float temp = bme.readTemperature();
        float humidity = bme.readHumidity();
        float pressure = bme.readPressure() / 100.0F;
        float bat_voltage = adc.readVoltage() * deviderRatio;

        // Create JSON payload
        String payload = "{";
        payload += "\"temperature\":" + String(temp) + ",";
        payload += "\"humidity\":" + String(humidity) + ",";
        payload += "\"pressure\":" + String(pressure) + ",";
        payload += "\"battery_voltage\":" + String(bat_voltage);
        payload += "}";

        Serial.println("Sending data: " + payload);

        int httpResponseCode = http.POST(payload);

        if (httpResponseCode > 0) {
            Serial.println("Response: " + http.getString());
        } else {
            Serial.println("Error on sending POST: " + String(httpResponseCode));
        }

        http.end();
    } else {
        Serial.println("WiFi Disconnected!");
    }
}

void loop() {
    // Nothing here, as ESP32 resets after deep sleep
}

