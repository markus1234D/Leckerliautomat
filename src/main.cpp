#include <Arduino.h>
// #include <AccelStepper.h>
#include "GuiWorker.h"
#include "MotorWorker.h"


// put function declarations here:
GuiWorker guiWorker;
MotorWorker motorWorker(20000, 8); // Initialize MotorWorker with motor pin 19, frequency 20kHz, resolution 8 bits
// AccelStepper stepper(AccelStepper::FULL4WIRE, 19,5,18,17);
 // Define the stepper motor pins

void setup() {
  Serial.begin(115200);
  Serial.println("Hello World!");
  guiWorker.init();

  guiWorker.onFireButtonClick([](int speed, int time) {
    Serial.printf("Fire button pressed with speed: %d, time: %d\n", speed, time);
    motorWorker.run(speed, time);
  });
  guiWorker.onMotorGo([](int speed) {
    Serial.printf("Motor go with speed: %d\n", speed);
    motorWorker.run(speed);
  });
  guiWorker.onMotorStop([]() {
    Serial.println("Motor stop");
    motorWorker.stop();
  });


}

void loop() {
  guiWorker.handleGui();
  // if (stepper.distanceToGo() != 0) {
  //   stepper.run(); // Run the stepper motor
  // }
  delay(2);
}
