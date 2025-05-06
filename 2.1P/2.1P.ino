#include <WiFiNINA.h>
#include "ThingSpeak.h"
#include "secrets.h"
#include <DHT.h>

#define DHTPIN 5
#define DHTTYPE DHT11

DHT dht(DHTPIN, DHTTYPE);
WiFiClient client;

unsigned long channelID = SECRET_CH_ID;
const char *writeAPIKey = SECRET_WRITE_APIKEY;

void setup() {
  Serial.begin(9600);
  while (!Serial);

  dht.begin();

  WiFi.disconnect();
  delay(1000);

  Serial.print("Connecting to WiFi: ");
  Serial.println(SECRET_SSID);

  WiFi.begin(SECRET_SSID, SECRET_PASS);

  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 20) {
    delay(1000);
    Serial.print(".");
    attempts++;
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\nWiFi connected.");
  } else {
    Serial.println("\nWiFi connection failed.");
  }

  ThingSpeak.begin(client);
}

void loop() {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("WiFi not connected. Attempting reconnection...");
    WiFi.disconnect();
    WiFi.begin(SECRET_SSID, SECRET_PASS);
    delay(10000);
    return;
  }

  float temp = dht.readTemperature();
  float hum = dht.readHumidity();

  if (isnan(temp) || isnan(hum)) {
    Serial.println("Failed to read from DHT sensor.");
    delay(60000);
    return;
  }

  Serial.print("Temperature: ");
  Serial.print(temp);
  Serial.print("°C, Humidity: ");
  Serial.print(hum);
  Serial.println("%");

  int alarmFlag = 0;
  if (temp > 35 || hum < 30) {
    alarmFlag = 1;
    Serial.println("Alarm triggered.");
  } else {
    Serial.println("Values within normal range.");
  }

  ThingSpeak.setField(1, temp);
  ThingSpeak.setField(2, hum);
  ThingSpeak.setField(3, alarmFlag);

  int response = ThingSpeak.writeFields(channelID, writeAPIKey);
  if (response == 200) {
    Serial.println("ThingSpeak update successful.");
  } else {
    Serial.print("ThingSpeak update failed. Error code: ");
    Serial.println(response);
  }

  delay(60000);
}
