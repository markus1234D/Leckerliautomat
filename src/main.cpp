#include <Arduino.h>
// #include <AccelStepper.h>
#include "GuiWorker.h"
#include "MotorWorker.h"
#include "Debug.h"


// put function declarations here:
GuiWorker guiWorker;
MotorWorker motorWorker(20000, 8); // Initialize MotorWorker with motor pin 19, frequency 20kHz, resolution 8 bits
// AccelStepper stepper(AccelStepper::FULL4WIRE, 19,5,18,17);
 // Define the stepper motor pins

void debugPrint(const char* format, ...) {
#ifdef DEBUG
    va_list args;
    va_start(args, format);
    writeDebugMessage("main", format, args);
    va_end(args);
#else
    (void)format;
#endif
}

void setup() {
  Serial.begin(115200);
  debugPrint("Hello World!");
  guiWorker.init();

  guiWorker.onFireButtonClick([](int speed, float time) {
    debugPrint("Fire button pressed with speed: %d, time: %.2f", speed, time);
    motorWorker.run(speed, time);
  });
  guiWorker.onMotorGo([](int speed) {
    debugPrint("Motor go with speed: %d", speed);
    motorWorker.run(speed);
  });
  guiWorker.onMotorStop([]() {
    debugPrint("Motor stop");
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
