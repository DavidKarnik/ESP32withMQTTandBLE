#include <Arduino.h>

#include <WiFi.h>
#include <PubSubClient.h> // knihovna https://registry.platformio.org/libraries/knolleary/PubSubClient/installation

#define BUTTON_PIN 21 // GIOP21 pin connected to button

const char* ssid = "SSID"; // SSID (WiFi name)
const char* password = "PASSWD"; //WIFI_password
const char* mqtt_server = "BrokerIP"; // my_mosquitto_broker_IP_address

WiFiClient espClient;
PubSubClient client(espClient);

String msg = "Turn LED ON";

// Button
int lastState = HIGH; // the previous state from the input pin
int currentState;     // the current reading from the input pin

void setup_wifi() {
  delay(10);
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);
  delay(1000);
  WiFi.disconnect();
  delay(1000);
  WiFi.begin(ssid, password);
  delay(1000);

  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void callback(char* topic, byte* message, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i=0;i<length;i++) {
    Serial.print((char)message[i]);
  }
  Serial.println();
}

void reconnect() {
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    if (client.connect("ESP32Client")) {
      Serial.println("connected");
      client.subscribe("topic",2);
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      delay(5000);
    }
  }
}

void setup() {
  Serial.begin(115200);
  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);

  // button
  pinMode(BUTTON_PIN, INPUT_PULLUP); // config GIOP21 as input pin and enable the internal pull-up resistor
  // LED
  // initialize digital pin GIOP18 as an output.
  pinMode(18, OUTPUT);
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  // Publish a message with QoS level 1
  //client.publish("topic", "Hello, World!", true, 1);

    // read the state of the switch/button:
  currentState = digitalRead(BUTTON_PIN);
  if(lastState == LOW && currentState == HIGH) {
    Serial.println("The state changed from LOW to HIGH");
    Serial.println("Message Published");

    client.publish("topic", msg.c_str());
    digitalWrite(18, HIGH); // turn the LED on
    delay(500);
  }

  digitalWrite(18, LOW);  // turn the LED off

  // save the last state
  lastState = currentState;

  delay(100);
}