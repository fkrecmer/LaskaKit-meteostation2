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
#include <ArduinoJson.h>
#include "esp_sleep.h"

#define WIFI_SSID       "<your_ssid>"
#define WIFI_PASSWORD   "<your_password>"
#define SERVER_URL      "<your-server:port>"

#define ADC_PIN         0                 
#define I2C_SDA         19
#define I2C_SCL         18
#define SENSOR_PWR_PIN  3
#define VOLTAGE_DIVIDER_RATIO  1.7693877551 
#define SLEEP_TIME_US   300000000ULL  // 5 minutes

Adafruit_BME280 bme;
ESP32AnalogRead adc;

bool connectToWiFi(unsigned long timeout = 15000) {
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    Serial.print("Connecting to WiFi...");
    unsigned long start = millis();
    while (WiFi.status() != WL_CONNECTED && millis() - start < timeout) {
        delay(500);
        Serial.print(".");
    }
    if (WiFi.status() == WL_CONNECTED) {
        Serial.println("\nConnected to WiFi");
        return true;
    } else {
        Serial.println("\nWiFi connection failed");
        return false;
    }
}

void sendData() {
    if (WiFi.status() == WL_CONNECTED) {
        HTTPClient http;
        http.begin(SERVER_URL);
        http.addHeader("Content-Type", "application/json");

        // Read sensors
        float temp = bme.readTemperature();
        float humidity = bme.readHumidity();
        float pressure = bme.readPressure() / 100.0F;
        float bat_voltage = adc.readVoltage() * VOLTAGE_DIVIDER_RATIO;

        // Build JSON
        StaticJsonDocument<200> doc;
        doc["temperature"] = temp;
        doc["humidity"] = humidity;
        doc["pressure"] = pressure;
        doc["battery_voltage"] = bat_voltage;

        String payload;
        serializeJson(doc, payload);

        Serial.println("Sending data: " + payload);

        int httpResponseCode = http.POST(payload);

        if (httpResponseCode > 0) {
            Serial.println("Response: " + http.getString());
        } else {
            Serial.println("Error sending POST: " + String(httpResponseCode));
        }

        http.end();
    } else {
        Serial.println("WiFi not connected, skipping send.");
    }
}

void setup() {
    Serial.begin(115200);

    pinMode(SENSOR_PWR_PIN, OUTPUT);
    digitalWrite(SENSOR_PWR_PIN, HIGH); // Power sensors

    Wire.begin(I2C_SDA, I2C_SCL);

    if (!bme.begin()) {
        Serial.println("BME280 not found");
        while (1) delay(1);
    }

    adc.attach(ADC_PIN);

    if (connectToWiFi()) {
        sendData();
    }

    Serial.println("Going to sleep...");
    WiFi.disconnect(true);
    WiFi.mode(WIFI_OFF);
    digitalWrite(SENSOR_PWR_PIN, LOW);

    esp_sleep_enable_timer_wakeup(SLEEP_TIME_US);
    delay(100);
    esp_deep_sleep_start();
}

void loop() {
    // Not used
}


