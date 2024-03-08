#include <Arduino.h>

#define BUTTON_PIN 21 // GIOP21 pin connected to button

// Variables will change:
int lastState = HIGH; // the previous state from the input pin
int currentState;     // the current reading from the input pin

void setup() {
  Serial.begin(9600);
  // Button
  // initialize the pushbutton pin as an pull-up input
  pinMode(BUTTON_PIN, INPUT_PULLUP);

  // LED
  // initialize digital pin GIOP18 as an output.
  pinMode(18, OUTPUT);
}

void loop() {
  // read the state of the switch/button:
  currentState = digitalRead(BUTTON_PIN);

  if(lastState == LOW && currentState == HIGH) {
      Serial.println("The state changed from LOW to HIGH");
      digitalWrite(18, HIGH); // turn the LED on
      delay(1000);             // wait for 1000 milliseconds
  }

  // save the last state
  lastState = currentState;
  digitalWrite(18, LOW);  // turn the LED off
  delay(100);
}