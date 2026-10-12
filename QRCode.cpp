/**
 *  QRCode
 *
 *  A quick example of generating a QR code.
 *
 *  This prints the QR code to the serial monitor as solid blocks. Each module
 *  is two characters wide, since the monospace font used in the serial monitor
 *  is approximately twice as tall as wide.
 *
 */
#include <Arduino.h>
#include <stdarg.h>
#include "QRCodeGenerator.h"
#include "src/Debug.h"

static void debugPrint(const char* format, ...) {
#ifdef DEBUG
    va_list args;
    va_start(args, format);
    writeDebugMessage("QRCode", format, args);
    va_end(args);
#else
    (void)format;
#endif
}

void setup() {
    Serial.begin(115200);

    // Start time
    uint32_t dt = millis();
  
    // Create the QR code
    QRCode qrcode;
    uint8_t qrcodeData[qrcode_getBufferSize(3)];
    qrcode_initText(&qrcode, qrcodeData, 3, 0, "192.168.115.105");
  
    // Delta time
    dt = millis() - dt;
    String output = "QR Code Generation Time: " + String(dt) + "\n";
    output += "size: " + String(qrcode.size) + "\n";

    // Top quiet zone
    output += "\n\n\n";

    for (uint8_t y = 0; y < qrcode.size; y++) {

        // Left quiet zone
        // Each horizontal module
        for (uint8_t x = 0; x < qrcode.size; x++) {

            // Print each module (UTF-8 \u2588 is a solid block)
            output += qrcode_getModule(&qrcode, x, y) ? "1" : "0";

        }

        output += "\n";
    }

    // Bottom quiet zone
    output += "\n\n\n\n";
    debugPrint("%s", output.c_str());
}

void loop() {

}
