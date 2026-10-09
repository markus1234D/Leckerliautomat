#include <Arduino.h>
#include <vector>
#include <WebSocketsServer.h>
#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include "DataWorker.h"

// #define DEBUG

#define WIFI_SSID                    "FRITZ!Mox"
#define WIFI_PASSWORD               "BugolEiz42"
// #define WIFI_SSID                    "ZenFone7 Pro_6535"
// #define WIFI_PASSWORD                "e24500606"
// #define WIFI_SSID                    "SM-Fritz"
// #define WIFI_PASSWORD                "47434951325606561069"

class GuiWorker {

public:
    // Constructor
    GuiWorker();
    void init();
    void handleGui();
    void onFireButtonClick(void (*callback)(int speed, int steps)) {
        fireButtonCallback = callback;
    }
    void onMotorGo(void (*callback)(int speed)) {
        motorGoCallback = callback;
    }
    void onMotorStop(void (*callback)()) {
        motorStopCallback = callback;
    }


private:
    // Private member variables
    void (*fireButtonCallback)(int speed, int steps) = nullptr;
    void (*motorGoCallback)(int speed) = nullptr;
    void (*motorStopCallback)() = nullptr;
    WebSocketsServer webSocketServer;
    AsyncWebServer server;

    // Private member functions
    void webSocketEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t length);
    String extractCommand(const String& input);
    int extractArgs(const String& input, std::vector<String>& argNames, std::vector<String>& args);
    void debugPrint(String str);
    void handleMessage(const String& message);
};

GuiWorker::GuiWorker() : webSocketServer(81), server(80) {
    // Constructor
}

void GuiWorker::debugPrint(String str) {
#ifdef DEBUG
    Serial.print("[debug]: ");
    Serial.println(str);
#endif
}

void GuiWorker::init() {
    //set fix IP address
    // IPAddress local_IP(192,168,178,42);
    // if (!WiFi.config(local_IP, WiFi.gatewayIP(), WiFi.subnetMask())) {
    //     Serial.println("STA Failed to configure");
    // }

    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    while (WiFi.status() != WL_CONNECTED) {
        delay(1000);
        debugPrint("Connecting to WiFi...");
    }
    debugPrint("Connected to WiFi");
    debugPrint("IP address: ");
    Serial.println(WiFi.localIP());     //TODO.


    // Start WebSocket-Server
    webSocketServer.begin();
    webSocketServer.onEvent([this](uint8_t num, WStype_t type, uint8_t * payload, size_t length) {
        this->webSocketEvent(num, type, payload, length);
    });
    // webSocketServer.onEvent(webSocketEvent);
    debugPrint("WebSocket server started.");

    server.on("/", HTTP_GET, [this](AsyncWebServerRequest *request){
        request->send(200, "text/html", DataWorker().getFileContent("/motor_control_gui.html"));
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
        debugPrint("No arguments found");
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
        debugPrint(String(num) + " Disconnected!");
        break;

    case WStype_CONNECTED:
        debugPrint(String(num) + " Connected!");
        webSocketServer.sendTXT(num, "Hello from ESP32!");  //TODO mit this-> verdeutlichen, dass es sich um die Instanz der Klasse handelt

        break;
    case WStype_TEXT:
        debugPrint(String(num) + " Message received: " + String((char*)payload));
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
    debugPrint("Command: " + command);
    debugPrint("NumArgs: " + String(numArgs));
    for (int i = 0; i < numArgs; i++) {
        debugPrint(argNames[i] + ": " + args[i]);
    }

    if (command == "fire") {
        int speed = args[0].toInt();
        int steps = args[1].toInt();
        debugPrint("Fire command received with speed: " + String(speed) + " and steps: " + String(steps));
        if(fireButtonCallback) {
            fireButtonCallback(speed, steps);
        } else {
            debugPrint("No callback set for fire button");
        }
    }
    else if (command == "motorGo") {
        int speed = args[0].toInt();
        debugPrint("MotorGo command received with speed: " + String(speed));
        if (motorGoCallback) {
            motorGoCallback(speed);
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
