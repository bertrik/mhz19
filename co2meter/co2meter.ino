#include <stdio.h>

#include "Arduino.h"
#include "mhz19.h"

#include "SoftwareSerial.h"

#include <ESP8266WiFi.h>
#include <WiFiManager.h>
#include <PubSubClient.h>

#define PIN_RX  D1
#define PIN_TX  D2

#define MQTT_HOST   "stofradar.nl"
#define MQTT_PORT   1883
#define MQTT_TOPIC  "bertrik/sensors/co2/mhz19"

static SoftwareSerial sensor(PIN_RX, PIN_TX);
static MHZ19 mhz19(&sensor);

static char esp_id[16];
static WiFiManager wifiManager;
static WiFiClient wifiClient;
static PubSubClient mqttClient(wifiClient);

#ifdef SEND_MQTT
// sends a value on the specified topic (retained), attempts to connect if not connected
static bool mqtt_send(const char *topic, const char *value, bool retained)
{
    bool result = false;
    if (!mqttClient.connected()) {
        Serial.print("Connecting MQTT...");
        mqttClient.setServer(MQTT_HOST, MQTT_PORT);
        result = mqttClient.connect(esp_id, topic, 0, retained, "offline");
        Serial.println(result ? "OK" : "FAIL");
    }
    if (mqttClient.connected()) {
        Serial.print("Publishing ");
        Serial.print(value);
        Serial.print(" to ");
        Serial.print(topic);
        Serial.print("...");
        result = mqttClient.publish(topic, value, retained);
        Serial.println(result ? "OK" : "FAIL");
    }
    return result;
}
#endif

void setup(void)
{
    Serial.begin(115200);
    Serial.println("MHZ19 ESP reader\n");

    sprintf(esp_id, "%08X", ESP.getChipId());
    Serial.print("ESP ID: ");
    Serial.println(esp_id);

    // initialize
    sensor.begin(MHZ19::BIT_RATE);

#ifdef SEND_MQTT
    Serial.println("Starting WIFI manager ...");
    wifiManager.setConfigPortalTimeout(120);
    wifiManager.autoConnect("ESP-MHZ19");
#endif
}

void loop()
{
    static unsigned long last_sent = 0;
    int co2, temp;

    unsigned long m = millis();
    if ((m - last_sent) > 5000) {
        if (mhz19.readCo2(&co2, &temp)) {
            Serial.printf("CO2 = %d ppm, Temperature = %d degC\n", co2, temp);

#ifdef SEND_MQTT
            // send over MQTT
            char message[16];
            snprintf(message, sizeof(message), "%d ppm", co2);
            if (!mqtt_send(MQTT_TOPIC, message, true)) {
                Serial.println("Restarting ESP...");
                ESP.restart();
            }
#endif
        }
        last_sent = m;
    }
    mqttClient.loop();
}
