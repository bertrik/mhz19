#include <stdio.h>

#include "Arduino.h"
#include "mhz19.h"

#include "SoftwareSerial.h"

#include <ESP8266WiFi.h>
#include <WiFiManager.h>
#include <PubSubClient.h>

#define PIN_RX  D1
#define PIN_TX  D2

#define MQTT_HOST   "mosquitto.space.revspace.nl"
#define MQTT_PORT   1883
#define MQTT_TOPIC  "revspace/sensors/co2/mhz19"

SoftwareSerial sensor(PIN_RX, PIN_TX);
WiFiManager wifiManager;
WiFiClient wifiClient;
PubSubClient mqttClient(wifiClient);

static char esp_id[16];

static bool exchange_command(uint8_t cmd, uint8_t data[], unsigned int timeout)
{
    // create command buffer
    uint8_t buf[9];
    int len = prepare_tx(cmd, data, buf, sizeof(buf));

    // send the command
    sensor.write(buf, len);

    // wait for response
    unsigned long start = millis();
    while ((millis() - start) < timeout) {
        if (sensor.available() > 0) {
            uint8_t b = sensor.read();
            if (process_rx(b, cmd, data)) {
                return true;
            }
        }
        yield();
    }

    return false;
}

static bool read_temp_co2(int *co2, int *temp)
{
    uint8_t data[] = { 0, 0, 0, 0, 0, 0 };
    bool result = exchange_command(0x86, data, 3000);
    if (result) {
        *co2 = (data[0] << 8) + data[1];
        *temp = data[2] - 40;
#if 1
        char raw[32];
        sprintf(raw, "RAW: %02X %02X %02X %02X %02X %02X", data[0], data[1], data[2], data[3],
                data[4], data[5]);
        Serial.println(raw);
#endif
    }
    return result;
}

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

void setup()
{
    Serial.begin(115200);
    Serial.println("MHZ19 ESP reader\n");

    sprintf(esp_id, "%08X", ESP.getChipId());
    Serial.print("ESP ID: ");
    Serial.println(esp_id);

    sensor.begin(9600);

    Serial.println("Starting WIFI manager ...");
    wifiManager.setConfigPortalTimeout(120);
    wifiManager.autoConnect("ESP-MHZ19");
}

void loop()
{
    static unsigned long last_sent = 0;
    int co2, temp;

    unsigned long m = millis();
    if ((m - last_sent) > 5000) {
        if (read_temp_co2(&co2, &temp)) {
            Serial.print("CO2:");
            Serial.println(co2, DEC);
            Serial.print("TEMP:");
            Serial.println(temp, DEC);

            // send over MQTT
            char message[16];
            snprintf(message, sizeof(message), "%d ppm", co2);
            if (!mqtt_send(MQTT_TOPIC, message, true)) {
                Serial.println("Restarting ESP...");
                ESP.restart();
            }
        }
        last_sent = m;
    }
    mqttClient.loop();
}
