#include <Arduino.h>

#include <WiFi.h>
#include <PubSubClient.h>

const char* ssid = "SSID"; // SSID (WiFi name)
const char* password = "PASSWD"; //WIFI_password
const char* mqtt_server = "BrokerIP"; // my_mosquitto_broker_IP_address

WiFiClient espClient;
PubSubClient client(espClient);

void setup() {
  // Connect to WiFi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
  }

  // Connect to MQTT broker
  client.setServer(mqtt_server, 1883);

  while (!client.connected()) {
    if (client.connect("ESP32Client")) {
      Serial.println("Connected to MQTT broker");
    } else {
      delay(1000);
    }
  }

  // Publish a message with QoS level 1
  client.publish("test_topic", "Hello, World!", 1);
}

void loop() {
  client.loop();
}