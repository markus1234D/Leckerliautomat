#ifndef MOTORWORKER_H
#define MOTORWORKER_H

#include <Arduino.h>
#include <stdarg.h>
#include "Debug.h"

#define InA 18 // Motor control pin A
#define InB 5 // Motor control pin B
#define En1 17  // Motor control pin Enable
#define MotorChannel 0  // PWM channel for motor control

class MotorWorker {
public:
    
    MotorWorker(uint32_t pwmFrequency = 20000, uint8_t pwmResolution = 8);
    void run(uint8_t duty, float timeSeconds);
    void run(uint8_t duty);
    void stop();

private:
    bool initialized;
    void debugPrint(const char* format, ...);
};

void MotorWorker::debugPrint(const char* format, ...) {
#ifdef DEBUG
    va_list args;
    va_start(args, format);
    writeDebugMessage("MotorWorker", format, args);
    va_end(args);
#else
    (void)format;
#endif
}

MotorWorker::MotorWorker(uint32_t pwmFrequency, uint8_t pwmResolution)
    : initialized(false) {
    if (ledcSetup(MotorChannel, pwmFrequency, pwmResolution) == 0) {
        debugPrint("MotorWorker: PWM setup failed");
        return;
    }
    ledcAttachPin(En1, MotorChannel);
    initialized = true;
}

void MotorWorker::run(uint8_t duty, float timeSeconds) {
    if (!initialized) {
        debugPrint("MotorWorker: PWM is not initialized");
        return;
    }
    debugPrint("MotorWorker: Running motor with duty cycle %d for %.1f seconds", duty, timeSeconds);
    ledcWrite(MotorChannel, duty);
    delay(static_cast<unsigned long>(timeSeconds) * 1000UL);    //TODO: asynchron machen
    ledcWrite(MotorChannel, 0);
}

void MotorWorker::run(uint8_t duty) {
    if (!initialized) {
        debugPrint("MotorWorker: PWM is not initialized");
        return;
    }
    debugPrint("MotorWorker: Running motor with duty cycle %d", duty);
    ledcWrite(MotorChannel, duty);
}

void MotorWorker::stop() {
    if (!initialized) {
        debugPrint("MotorWorker: PWM is not initialized");
        return;
    }
    debugPrint("MotorWorker: Stopping motor");
    ledcWrite(MotorChannel, 0);
}

#endif // MOTORWORKER_H
