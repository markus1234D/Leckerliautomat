#include <Arduino.h>
#include <vector>
#include <WebSocketsServer.h>
#include <WiFi.h>
#include <ESPAsyncWebServer.h>

#define DEBUG

// #define WIFI_SSID                    "FRITZ!Mox"
// #define WIFI_PASSWORD               "BugolEiz42"
#define WIFI_SSID                    "ZenFone7 Pro_6535"
#define WIFI_PASSWORD                "e24500606"
// #define WIFI_SSID                    "SM-Fritz"
// #define WIFI_PASSWORD                "47434951325606561069"

class GuiWorker {

public:
    // Constructor
    GuiWorker();
    void init();
    void handleGui();
    String getHtml();
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
    Serial.println("GuiWorker constructor called");
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
        Serial.println("Connecting to WiFi...");
    }
    debugPrint("Connected to WiFi");
    debugPrint("IP address: ");
    Serial.println(WiFi.localIP());


    // Start WebSocket-Server
    webSocketServer.begin();
    webSocketServer.onEvent([this](uint8_t num, WStype_t type, uint8_t * payload, size_t length) {
        this->webSocketEvent(num, type, payload, length);
    });
    // webSocketServer.onEvent(webSocketEvent);
    debugPrint("WebSocket server started.");

    server.on("/", HTTP_GET, [this](AsyncWebServerRequest *request){
        request->send(200, "text/html", this->getHtml());
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
        Serial.printf("[%u] Disconnected!\n", num);
        break;

    case WStype_CONNECTED:
        Serial.printf("[%u] Connected!\n", num);
        webSocketServer.sendTXT(num, "Hello from ESP32!");

        break;
    case WStype_TEXT:
        Serial.printf("[%u] Received text: %s\n", num, payload);
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
        // debugPrint("Fire command received with speed: " + String(speed) + " and steps: " + String(steps));
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

String GuiWorker::getHtml() {
    return R"rawliteral(
    
<!DOCTYPE html>
<html lang="de">
<head>
  <meta charset="UTF-8">
  <title>GUI mit Grid</title>
  <style>
    body {
      margin: 0;
      font-family: Arial, sans-serif;
    }
    h1 {
      text-align: center;
    }
    .row-container {
      display: flex; /* Flexbox aktivieren */
      flex-wrap: wrap;
      gap: 20px;
      @media (max-width: 600px) {
        flex-direction: column;
      }
      flex-direction: row;
      padding: 20px;
      column-gap: 20px; /* Abstand zwischen den Spalten */
      justify-content: center;
      text-align: center;
    }

    .column-container {
      flex: 1; /* Gleichmäßige Breite für alle Spalten */
      display: flex;
      flex-direction: column; /* Spaltenanordnung */
      border-color: #4CAF50;
      border-style: solid;
      border-width: 2px;
      color: white;
      text-align: center;
      padding: 20px;
      border-radius: 8px;
      align-items: center;
      justify-content: center;
      width: 100%;
    }
    .slider {
      width: 60%;
      margin: 10px 0;
      min-width: 200px; /* Mindestbreite für den Slider */
    }
    .number-input {
      width: 100px; /* Breite für die Eingabefelder */
      margin: 10px 0;
      max-width: 80px; /* Maximale Breite für die Eingabefelder */
    }
  </style>
</head>
<body>
  <h1>Leckerliautomat GUI</h1>
  <div class="row-container">
    <div class="column-container">
      <h2>Speed Control</h2>
      <div class="row-container">
        <label for="speedSlider" id="minSpeedLabel">0</label>
        <input type="range" id="speedSlider" class="slider" min="0" max="1000" aria-label="Speed Slider" >
        <label for="speedSlider" id="maxSpeedLabel">1000</label>
      </div>
      <label for="speedSlider" id="speedSliderValue">500</label>
      <div class="row-container">
        <div class="row-container">
          <button class="button" id="stopGoBtn">stop/go</button>
        </div>
      </div>
    </div>

    <div class="column-container">
      <h3>Step Control</h3>
      <div class="row-container">
        <div class="column-container">
          <label for="minStepInput">min</label>
          <input type="number" class="number-input" id="minStepInput" value="-1000">
        </div>
        <div class="column-container">
          <label for="maxStepInput">max</label>
          <input type="number" class="number-input" id="maxStepInput" value="1000">
        </div>
      </div>
      <div class="row-container">
        <label for="stepSlider" id="minStepLabel">0</label>
        <input type="range" id="stepSlider" class="slider" min="-1000" max="1000" aria-label="Step Slider">
        <label for="stepSlider" id="maxStepLabel">500</label>
      </div>
      <label for="stepSlider" id="stepSliderValue">500</label>
      <div class="row-container">
        <button class="button" id="resetBtn">Reset</button>
        <button class="button" id="fireBtn">Fire</button>
      </div>
    </div>
  </div>

    <script>
        // communication stuff
        //protokol: command?arg1=value1&arg2=value2...
        var ws = new WebSocket('ws://' + window.location.hostname + ':81');

        ws.onopen = function() {
            console.log("Websocket connected");
        };
        ws.onmessage = function (evt) {
            const command = extractCommand(evt.data);
            const { argNames, args } = extractArgs(evt.data);
            console.log("Command: " + command);
            console.log("ArgNames: " + argNames);
            console.log("Args: " + args);
        }

        function extractCommand(input) {
            const pos = input.indexOf('?');
            if (pos !== -1) {
                return input.substring(0, pos);
            } else {
                return input; // Wenn kein '?' gefunden wird, ist der ganze String der Command
            }
        }

        function extractArgs(input) {
            const argNames = [];
            const args = [];
            
            const pos = input.indexOf('?');
            if (pos === -1) return { argNames, args }; // Falls kein '?' vorhanden ist, keine Argumente

            const query = input.substring(pos + 1);
            const pairs = query.split('&');

            pairs.forEach(pair => {
                const [name, value] = pair.split('=');
                if (name && value) {
                    argNames.push(name);
                    args.push(value);
                }
            });
            return { argNames, args };
        }
        // end communication stuff

        const speedSlider = document.getElementById("speedSlider");
        const minStepInput = document.getElementById("minStepInput");
        const maxStepInput = document.getElementById("maxStepInput");
        const stepSlider = document.getElementById("stepSlider");
        const minStepLabel = document.getElementById("minStepLabel");
        const maxStepLabel = document.getElementById("maxStepLabel");
        const stepSliderValue = document.getElementById("stepSliderValue");
        const speedSliderValue = document.getElementById("speedSliderValue");
        const stopGoButton = document.getElementById("stopGoBtn");
        const resetButton = document.getElementById("resetBtn");
        const fireButton = document.getElementById("fireBtn");
        stopGoButton.addEventListener("mousedown", motorGo);
        stopGoButton.addEventListener("mouseup", motorStop);
        resetButton.addEventListener("click", resetSlider);
        fireButton.addEventListener("click", fire);
        stepSlider.min = minStepInput.value;
        stepSlider.max = maxStepInput.value;
        minStepLabel.textContent = minStepInput.value;
        maxStepLabel.textContent = maxStepInput.value;
        stepSliderValue.textContent = stepSlider.value;
        speedSliderValue.textContent = speedSlider.value;

        function updateStepSliderLimits() {
            stepSlider.min = minStepInput.value;
            stepSlider.max = maxStepInput.value;
            minStepLabel.textContent = minStepInput.value;
            maxStepLabel.textContent = maxStepInput.value;
            stepSlider.value = Math.min(stepSlider.value, maxStepInput.value);
            stepSliderValue.textContent = stepSlider.value;
        }

        function resetSlider() {
            minStepInput.value = -1000;
            maxStepInput.value = 1000;
            updateStepSliderLimits();
        }

        function stopAndGo() {
            // Implement stop/go functionality here
            console.log("Stop/Go button clicked");
        }

        minStepInput.addEventListener("change", updateStepSliderLimits);
        maxStepInput.addEventListener("change", updateStepSliderLimits);
        speedSlider.addEventListener("input", () => {
            speedSliderValue.textContent = speedSlider.value;
        });
        stepSlider.addEventListener("input", () => {
            stepSliderValue.textContent = stepSlider.value;
        });

        function fire() {
            const speed = speedSlider.value;
            const steps = stepSlider.value;
            const url = `fire?speed=${speed}&steps=${steps}`;
            console.log("ws send: " + url);
            ws.send(url);
        }

        function motorGo() {
            const speed = speedSlider.value;
            const steps = stepSlider.value;
            const url = `motorGo?speed=${speed}`;
            console.log("ws send: " + url);
            ws.send(url);
        }
        function motorStop() {
            const url = "motorStop";
            console.log("ws send: " + url);
            ws.send(url);
        }
        
    </script>
</body>
</html>


)rawliteral";

}
