#ifndef DEBUG_H
#define DEBUG_H

#include <Arduino.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

inline void writeDebugMessage(const char* tag, const char* format, va_list args) {
    va_list formatArgs;
    va_copy(formatArgs, args);
    int requiredLength = vsnprintf(nullptr, 0, format, formatArgs);
    va_end(formatArgs);

    if (requiredLength < 0) {
        Serial.printf("[%s]: Failed to format debug message\n", tag);
        return;
    }

    char* message = static_cast<char*>(malloc(static_cast<size_t>(requiredLength) + 1));
    if (message == nullptr) {
        Serial.printf("[%s]: Failed to allocate debug message buffer\n", tag);
        return;
    }

    int messageLength = vsnprintf(message, static_cast<size_t>(requiredLength) + 1, format, args);
    if (messageLength != requiredLength) {
        free(message);
        Serial.printf("[%s]: Failed to format debug message\n", tag);
        return;
    }

    Serial.printf("[%s]: ", tag);
    Serial.write(reinterpret_cast<const uint8_t*>(message), static_cast<size_t>(messageLength));
    Serial.println();
    free(message);
}

#endif
