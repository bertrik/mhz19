#include <stdio.h>

#include "Arduino.h"
#include "mhz19.h"

#include "SoftwareSerial.h"

#include <ESP8266WiFi.h>
#include <WiFiManager.h>
#include <PubSubClient.h>

#define PIN_RX  D1
#define PIN_TX  D2

#define MQTT_HOST   "revspace.nl"
#define MQTT_PORT   1883
#define MQTT_TOPIC  "revspace/sensors/co2/mhz19"

SoftwareSerial sensor(PIN_RX, PIN_TX);
WiFiManager wifiManager;
WiFiClient wifiClient;
PubSubClient mqttClient(wifiClient);

static char esp_id[16];

static bool exchange_command(uint8_t cmd, uint8_t data[], int timeout)
{
    // create command buffer
    uint8_t buf[9];
    int len = prepare_tx(cmd, data, buf, sizeof(buf));

    // send the command
    sensor.write(buf, len);

    // wait for response
    long start = millis();
    while ((millis() - start) < timeout) {
        if (sensor.available() > 0) {
            uint8_t b = sensor.read();
            if (process_rx(b, cmd, data)) {
                return true;
            }
        }
    }

    return false;
}

static bool read_temp_co2(int *co2, int *temp)
{
    uint8_t data[] = {0, 0, 0, 0, 0, 0};
    bool result = exchange_command(0x86, data, 3000);
    if (result) {
        *co2 = (data[0] << 8) + data[1];
        *temp = data[2] - 40;
#if 1
        char raw[32];
        sprintf(raw, "RAW: %02X %02X %02X %02X %02X %02X", data[0], data[1], data[2], data[3], data[4], data[5]);
        Serial.println(raw);
#endif
    }
    return result;
}

static void connect_mqtt(const char *host, int port)
{
    char esp_id[10];

    if (!mqttClient.connected()) {
        sprintf(esp_id, "%08X", ESP.getChipId());
        Serial.print("Connecting to MQTT as ");
        Serial.println(esp_id);
        mqttClient.setServer(host, port);
        mqttClient.connect(esp_id);
    }
}

void setup()
{
    Serial.begin(115200);

    sprintf(esp_id, "%08X", ESP.getChipId());
    Serial.print("ESP ID: ");
    Serial.println(esp_id);

    sensor.begin(9600);

    Serial.println("Starting WIFI manager ...");
    wifiManager.autoConnect("ESP-CO2");
}

void loop()
{
    int co2, temp;
    if (read_temp_co2(&co2, &temp)) {
        Serial.print("CO2:");
        Serial.println(co2, DEC);
        Serial.print("TEMP:");
        Serial.println(temp, DEC);

        // maintain MQTT connection
        if (!mqttClient.connected()) {
            connect_mqtt(MQTT_HOST, MQTT_PORT);
        }

        // try to publish it
        if (mqttClient.connected()) {
            char value[16];
            sprintf(value, "%d", co2);
            Serial.print("Publishing...");
            int result = mqttClient.publish(MQTT_TOPIC, value, true);
            Serial.println(result ? "OK" : "FAIL");
        } else {
            Serial.println("MQTT not connected!");
        }
    } else {
        Serial.println("failed to read CO2 sensor!");
    }
    delay(5000);
}


