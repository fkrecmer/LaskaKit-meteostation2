#include <Wire.h>
#include <Adafruit_BME280.h>
#include <ESP32AnalogRead.h>      // ESP32AnalogRead by madhephaestus https://github.com/madhephaestus/ESP32AnalogRead

#define ADC_PIN 0                 // ADC pin on LaskaKit Meteo mini
#define SDA 19
#define SCL 18
#define PIN_ON 3
#define deviderRatio 1.7693877551 // Voltage divider ratio on ADC pin 1M + 1.3MOhm

Adafruit_BME280 bme; // Create an instance of the object for BME280
ESP32AnalogRead adc;

void setup() {
  Serial.begin(115200);
  while (!Serial)
  {
    ; // waiting for Serial port
  }

  // for version over 3.5 need to turn uSUP ON
  pinMode(PIN_ON, OUTPUT);      // Set EN pin for uSUP stabilizer as output
  digitalWrite(PIN_ON, HIGH);   // Turn on the uSUP power

  Wire.begin(SDA, SCL);

  if (!bme.begin()) // Initialization of BME280
  {
    Serial.println("BME280 not found");
    Serial.println("Check connections");
    while (1) delay(1);
  }
}

void loop() {
  adc.attach(ADC_PIN);
  
  // Getting data from BME280
  float temp = bme.readTemperature();
  float humidity = bme.readHumidity();
  float pressure    = bme.readPressure() / 100.0F;
  float bat_voltage = adc.readVoltage()*deviderRatio;

  Serial.println("Temperature: " + String(temp) + " degC");
  Serial.println("Humidity: " + String(humidity) + "% rH");
  Serial.println("Pressure: " + String(pressure) + " hPa");
  Serial.println("Battery voltage: " + String(bat_voltage) + "V");
  Serial.println();
  delay(1000);
}
