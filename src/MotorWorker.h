#ifndef MOTORWORKER_H
#define MOTORWORKER_H

#include <Arduino.h>
#include <stdarg.h>
#include "Debug.h"

#define In1 18 // Motor control pin A
#define In2 5 // Motor control pin B
#define EnA 17  // Motor control pin Enable
#define MotorChannel 0  // PWM channel for motor control

class MotorWorker {
public:
    
    MotorWorker(uint32_t pwmFrequency = 20000, uint8_t pwmResolution = 8);
    void run(int32_t duty, float timeSeconds);
    void run(int32_t duty);
    void stop();

private:
    bool initialized;
    uint32_t maxDuty;
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
    : initialized(false), maxDuty(0) {
    if (pwmResolution == 0 || pwmResolution > 20) {
        debugPrint("MotorWorker: Invalid PWM resolution: %u", pwmResolution);
        return;
    }
    maxDuty = (1UL << pwmResolution) - 1;
    if (ledcSetup(MotorChannel, pwmFrequency, pwmResolution) == 0) {
        debugPrint("PWM setup failed");
        return;
    }
    ledcAttachPin(EnA, MotorChannel);

    pinMode(In1, OUTPUT);
    pinMode(In2, OUTPUT);
    digitalWrite(In1, LOW);
    digitalWrite(In2, LOW);
    initialized = true;
}

void MotorWorker::run(int32_t duty, float timeSeconds) {
    if (!initialized) {
        debugPrint("MotorWorker: PWM is not initialized");
        return;
    }
    bool reverse = duty < 0;
    // PWM needs a non-negative duty; widen before negating to safely handle INT32_MIN.
    uint32_t dutyMagnitude = static_cast<uint32_t>(
        reverse ? -static_cast<int64_t>(duty) : duty);
    // Keep the PWM value within the configured resolution.
    dutyMagnitude = min(dutyMagnitude, maxDuty);

    debugPrint("Running motor with duty cycle %ld for %.1f seconds",
               static_cast<long>(duty), timeSeconds);
    digitalWrite(In1, dutyMagnitude == 0 ? LOW : (reverse ? LOW : HIGH));
    digitalWrite(In2, dutyMagnitude == 0 ? LOW : (reverse ? HIGH : LOW));
    
    ledcWrite(MotorChannel, dutyMagnitude);
    delay(static_cast<unsigned long>(timeSeconds * 1000.0f));    //TODO: asynchron machen
    ledcWrite(MotorChannel, 0);
    digitalWrite(In1, LOW);
    digitalWrite(In2, LOW);
}

void MotorWorker::run(int32_t duty) {
    if (!initialized) {
        debugPrint("MotorWorker: PWM is not initialized");
        return;
    }
    bool reverse = duty < 0;
    // PWM needs a non-negative duty; widen before negating to safely handle INT32_MIN.
    uint32_t dutyMagnitude = static_cast<uint32_t>(
        reverse ? -static_cast<int64_t>(duty) : duty);
    // Keep the PWM value within the configured resolution.
    dutyMagnitude = min(dutyMagnitude, maxDuty);
    debugPrint("Running motor with duty cycle %ld", static_cast<long>(duty));
    digitalWrite(In1, dutyMagnitude == 0 ? LOW : (reverse ? LOW : HIGH));
    digitalWrite(In2, dutyMagnitude == 0 ? LOW : (reverse ? HIGH : LOW));
    ledcWrite(MotorChannel, dutyMagnitude);
}

void MotorWorker::stop() {
    if (!initialized) {
        debugPrint("MotorWorker: PWM is not initialized");
        return;
    }
    debugPrint("Stopping motor");
    ledcWrite(MotorChannel, 0);
    digitalWrite(In1, LOW);
    digitalWrite(In2, LOW);
}

#endif // MOTORWORKER_H
