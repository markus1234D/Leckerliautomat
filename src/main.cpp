#include <Arduino.h>
// #include <AccelStepper.h>
#include "GuiWorker.h"
#include "MotorWorker.h"
#include "Debug.h"


#define DEBUG
#define pwmFrequency 20000 // Set the PWM frequency to 20 kHz
#define pwmResolution 8   // Set the PWM resolution to 8 bits

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
  guiWorker.init(pwmResolution);

  guiWorker.onFireButtonClick([](int32_t duty, float time) {
    debugPrint("Fire button pressed with duty: %ld, time: %.2f", static_cast<long>(duty), time);
    motorWorker.run(duty, time);
  });
  guiWorker.onMotorGo([](int32_t duty) {
    debugPrint("Motor go with duty: %ld", static_cast<long>(duty));
    motorWorker.run(duty);
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
