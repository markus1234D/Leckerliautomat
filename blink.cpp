#include <Arduino.h>
#define LED_BUILTIN 8 // Internal LED pin is 8 as per schematic

// the setup function runs once when you press reset or power the board
void setup() {
  // initialize digital pin LED_BUILTIN as an output.
  pinMode(LED_BUILTIN, OUTPUT);
}

// the loop function runs over and over again forever
void loop() {
  digitalWrite(LED_BUILTIN, LOW);   // turn the LED on (LOW because the LED is inverted)
  delay(2000);                      // wait for two seconds
  digitalWrite(LED_BUILTIN, HIGH);  // turn the LED off by making the voltage HIGH
  delay(1000);                      // wait for a second
}