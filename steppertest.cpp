#include <AccelStepper.h>

// Define the stepper motor interface type and pins
AccelStepper stepper(AccelStepper::FULL4WIRE, 19,5,18,17);

void setup() {
    // Set the maximum speed and acceleration for the stepper motor
    stepper.setMaxSpeed(1000);
    stepper.setAcceleration(500);
}

void loop() {
    // Move the stepper motor to a target position
    stepper.moveTo(2000); // Target position in steps
    stepper.runToPosition(); // Blocking call to move to the target position

    delay(1000); // Wait for 1 second

    // Move back to the starting position
    stepper.moveTo(0);
    stepper.runToPosition();

    delay(1000); // Wait for 1 second
}