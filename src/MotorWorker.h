#ifndef MOTORWORKER_H
#define MOTORWORKER_H

#include <Arduino.h>

class MotorWorker {
public:
    MotorWorker(uint8_t motorPin, uint32_t pwmFrequency = 20000, uint8_t pwmResolution = 8);
    void run(uint8_t duty, uint32_t timeSeconds);
    void run(uint8_t duty);
    void stop();

private:
    bool initialized;
};

MotorWorker::MotorWorker(uint8_t motorPin, uint32_t pwmFrequency, uint8_t pwmResolution)
    : initialized(false) {
    if (ledcSetup(0, pwmFrequency, pwmResolution) == 0) {
        Serial.println("MotorWorker: PWM setup failed");
        return;
    }
    ledcAttachPin(motorPin, 0);
    initialized = true;
}

void MotorWorker::run(uint8_t duty, uint32_t timeSeconds) {
    if (!initialized) {
        Serial.println("MotorWorker: PWM is not initialized");
        return;
    }
    ledcWrite(0, duty);
    delay(static_cast<unsigned long>(timeSeconds) * 1000UL);    //TODO: asynchron machen
    ledcWrite(0, 0);
}

void MotorWorker::run(uint8_t duty) {
    if (!initialized) {
        Serial.println("MotorWorker: PWM is not initialized");
        return;
    }
    ledcWrite(0, duty);
}

void MotorWorker::stop() {
    if (!initialized) {
        Serial.println("MotorWorker: PWM is not initialized");
        return;
    }
    ledcWrite(0, 0);
}

#endif // MOTORWORKER_H
