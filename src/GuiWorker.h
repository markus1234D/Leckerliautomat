#include <Arduino.h>
#include <vector>
#include <WebSocketsServer.h>
#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <stdarg.h>
#include "Debug.h"
#include "DataWorker.h"

#define DEBUG

#define WIFI_SSID                    "FRITZ!Mox"
#define WIFI_PASSWORD               "BugolEiz42"
// #define WIFI_SSID                    "ZenFone7 Pro_6535"
// #define WIFI_PASSWORD                "e24500606"
// #define WIFI_SSID                    "SM-Fritz"
// #define WIFI_PASSWORD                "47434951325606561069"
// #define WIFI_SSID                    "Pixel_8965"
// #define WIFI_PASSWORD                "12345678"

class GuiWorker {

public:
    // Constructor
    GuiWorker();
    void init(uint8_t pwmResolution);
    void handleGui();
    void debugPrint(const char* format, ...);
    void onFireButtonClick(void (*callback)(int32_t duty, float seconds)) {
        fireButtonCallback = callback;
    }
    void onMotorGo(void (*callback)(int32_t duty)) {
        motorGoCallback = callback;
    }
    void onMotorStop(void (*callback)()) {
        motorStopCallback = callback;
    }


private:
    // Private member variables
    void (*fireButtonCallback)(int32_t duty, float seconds) = nullptr;
    void (*motorGoCallback)(int32_t duty) = nullptr;
    void (*motorStopCallback)() = nullptr;
    WebSocketsServer webSocketServer;
    AsyncWebServer server;
    uint32_t minDuty = 0;
    uint32_t maxDuty = 0;

    // Private member functions
    void webSocketEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t length);
    String extractCommand(const String& input);
    int extractArgs(const String& input, std::vector<String>& argNames, std::vector<String>& args);
    void handleMessage(const String& message);
};

GuiWorker::GuiWorker() : webSocketServer(81), server(80) {
    // Constructor
}

void GuiWorker::debugPrint(const char* format, ...) {
#ifdef DEBUG
    va_list args;
    va_start(args, format);
    writeDebugMessage("GuiWorker", format, args);
    va_end(args);
#else
    (void)format;
#endif
}

void GuiWorker::init(uint8_t pwmResolution) {
    if (pwmResolution == 0 || pwmResolution > 20) {
        debugPrint("Invalid PWM resolution: %u", pwmResolution);
        return;
    }
    maxDuty = (1UL << pwmResolution) - 1;

    //set fix IP address
    // IPAddress local_IP(192,168,178,42);
    // if (!WiFi.config(local_IP, WiFi.gatewayIP(), WiFi.subnetMask())) {
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    while (WiFi.status() != WL_CONNECTED) {
        delay(1000);
        debugPrint("Connecting to WiFi...");
    }
    debugPrint("Connected to WiFi");
    debugPrint("IP address: %s", WiFi.localIP().toString().c_str());


    // Start WebSocket-Server
    webSocketServer.begin();
    webSocketServer.onEvent([this](uint8_t num, WStype_t type, uint8_t * payload, size_t length) {
        this->webSocketEvent(num, type, payload, length);
    });
    // webSocketServer.onEvent(webSocketEvent);
    debugPrint("WebSocket server started.");

    server.on("/", HTTP_GET, [this](AsyncWebServerRequest *request){
        String page = DataWorker().getFileContent("/motor_control_gui.html");
        page.replace("{{MIN_DUTY}}", String(minDuty));
        page.replace("{{MAX_DUTY}}", String(maxDuty));
        request->send(200, "text/html", page);
    });
    server.begin();
}

String GuiWorker::extractCommand(const String& input) {
    int pos = input.indexOf('?');
    if (pos != -1) {
        return input.substring(0, pos);
    } else {
        return input; // Wenn kein '?' gefunden wird, ist der ganze String der Command
    }
}

// Funktion, um die Argumentnamen und -werte zu extrahieren
// fails if input is not in the form "command?arg1=val1&arg2=val2&..."
int GuiWorker::extractArgs(const String& input, std::vector<String>& argNames, std::vector<String>& args) {
    // debugPrint("Extracting arguments");
    int pos = input.indexOf('?');
    if (pos == -1) {
        // debugPrint("No arguments found");
        return 0; // Falls kein '?' vorhanden ist, keine Argumente
    }
    String query = input.substring(pos + 1);
    int start = 0;
    int end;
    int len = 0;

    while ((end = query.indexOf('&', start)) != -1) {
        String pair = query.substring(start, end);
        int equalPos = pair.indexOf('=');

        if (equalPos != -1) {
            len++;
            argNames.push_back(pair.substring(0, equalPos));
            args.push_back(pair.substring(equalPos + 1));
        }
        start = end + 1;
    }

    // Letztes Paar verarbeiten (nach dem letzten '&')
    String pair = query.substring(start);
    int equalPos = pair.indexOf('=');

    if (equalPos != -1) {
        len++;
        argNames.push_back(pair.substring(0, equalPos));
        args.push_back(pair.substring(equalPos + 1));
    }
    return len;
}

void GuiWorker::webSocketEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t length) {
    switch (type) {
    case WStype_DISCONNECTED:
        debugPrint("%d Disconnected!", num);
        break;

    case WStype_CONNECTED:
        debugPrint("%d Connected!", num);
        webSocketServer.sendTXT(num, "Hello from ESP32!");  //TODO mit this-> verdeutlichen, dass es sich um die Instanz der Klasse handelt

        break;
    case WStype_TEXT:
        debugPrint("%d Message received: %s", num, (char*)payload);
        String receivedMessage = String((char*)payload);
        handleMessage(receivedMessage);
        break;
    }
}

void GuiWorker::handleGui() {
    webSocketServer.loop();
}

void GuiWorker::handleMessage(const String& message) {
    String command = extractCommand(message);
    std::vector<String> argNames;
    std::vector<String> args;
    int numArgs = extractArgs(message, argNames, args);
    // debugPrint("Command: %s", command.c_str());
    // debugPrint("NumArgs: %d", numArgs);
    for (int i = 0; i < numArgs; i++) {
        debugPrint("%s: %s", argNames[i].c_str(), args[i].c_str());
    }

    if (command == "fire") {
        if (numArgs < 2) {
            debugPrint("Fire command requires duty and time arguments");
            return;
        }
        int32_t duty = args[0].toInt();
        float time = args[1].toFloat();
        debugPrint("Fire command received with duty: %ld and time: %.2f",
                   static_cast<long>(duty), time);
        if(fireButtonCallback) {
            fireButtonCallback(duty, time);
        } else {
            debugPrint("No callback set for fire button");
        }
    }
    else if (command == "motorGo") {
        if (numArgs < 1) {
            debugPrint("MotorGo command requires a duty argument");
            return;
        }
        int32_t duty = args[0].toInt();
        debugPrint("MotorGo command received with duty: %ld", static_cast<long>(duty));
        if (motorGoCallback) {
            motorGoCallback(duty);
        } else {
            debugPrint("No callback set for motor go");
        }
    }
    else if (command == "motorStop") {
        debugPrint("MotorStop command received");
        if (motorStopCallback) {
            motorStopCallback();
        } else {
            debugPrint("No callback set for motor stop");
        }
    }
}
