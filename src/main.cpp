#include <Arduino.h>
#include <AccelStepper.h>
#include "GuiWorker.h"

// put function declarations here:
GuiWorker guiWorker;
AccelStepper stepper(AccelStepper::FULL4WIRE, 19,5,18,17);
 // Define the stepper motor pins

void setup() {
  Serial.begin(115200);
  Serial.println("Hello World!");
  stepper.setMaxSpeed(1000); // Set the maximum speed of the stepper motor
  stepper.setAcceleration(1000); // Set the acceleration of the stepper motor
  stepper.setCurrentPosition(0); // Set the current position to 0 
  guiWorker.init();

  guiWorker.onFireButtonClick([](int speed, int steps) {
    Serial.printf("Fire button pressed with speed: %d, steps: %d\n", speed, steps);
    stepper.setMaxSpeed(speed);
    stepper.moveTo(steps + stepper.currentPosition()); // Move the stepper motor to the specified number of steps
  });

}

void loop() {
  guiWorker.handleGui();
  if (stepper.distanceToGo() != 0) {
    stepper.run(); // Run the stepper motor
  }
  delay(2);
}
