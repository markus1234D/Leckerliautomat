#include <Arduino.h>
#include <vector>
#include <WebSocketsServer.h>
#include <WiFi.h>
#include <ESPAsyncWebServer.h>

#define DEBUG

#define WIFI_SSID                    "FRITZ!Mox"
#define WIFI_PASSWORD               "BugolEiz42"
// #define WIFI_SSID                    "ZenFone7 Pro_6535"
// #define WIFI_PASSWORD                "e24500606"
// #define WIFI_SSID                    "SM-Fritz"
// #define WIFI_PASSWORD                "47434951325606561069"

class GuiWorker {

    typedef void (*valChangeFunc)(int attribute);   

public:
    // Constructor
    GuiWorker();
    void init();
    void handleGui();
    String getHtml();
    


private:
    // Private member variables
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
        // String response = "ESP32 received: " + receivedMessage;
        // webSocketServer.sendTXT(num, receivedMessage.c_str());
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

    if (command == "fire") {
        int speed = 0;
        int steps = 0;
        for (int i = 0; i < numArgs; i++) {
            if (argNames[i] == "speed") {
                speed = args[i].toInt();
            } else if (argNames[i] == "steps") {
                steps = args[i].toInt();
            }
        }
        debugPrint("Speed: " + String(speed));
        debugPrint("Steps: " + String(steps));
    }
}

String GuiWorker::getHtml() {
    return R"rawliteral(
    
<!DOCTYPE html>
<html lang="de">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Slider GUI</title>
    <style>
        body {
            background-color: rgb(33, 33, 33);
            color: lightblue;
            font-family: Arial, sans-serif;
        }
        .container {
            padding: 20px;
        }
        .slider-container {
            display: flex;
            align-items: center;
            gap: 10px;
        }
        .slider {
            flex-grow: 1;
        }
        .minMax {
            display: flex;
            align-items: center;
            gap: 10px;
            margin-bottom: 10px;
        }
        .button {
            padding: 5px 10px;
            background-color: lightblue;
            border: none;
            cursor: pointer;
        }
        input[type="number"] {
            width: 50px;
        }
    </style>
</head>
<body>
<h1>Leckerliautomat GUI</h1>
    <div class="container">
        <h2>Speed</h2>
        <div class="slider-container">
            <span id="minSpeedLabel">0</span>
            <input type="range" id="speedSlider" class="slider" min="0" max="10">
            <span id="maxSpeedLabel">10</span>
            <span id="speedSliderValue">0</span>
        </div>
    </div>

    <div class="container">
        <h2>Steps</h2>
        <div class="minMax">
            <label for="minStepInput">min</label>
            <input type="number" id="minStepInput" value="0">
            <label for="maxStepInput">max</label>
            <input type="number" id="maxStepInput" value="10">
        </div>

        <div class="slider-container">
            <span id="minStepLabel">0</span>
            <input type="range" id="stepSlider" class="slider" min="0" max="10">
            <span id="maxStepLabel">10</span>
            <label id="stepSliderValue">0</label>
        </div>

        <button class="button" onclick="resetSlider()">Reset</button>
        <button class="button" onclick="fire()">Fire</button>
    </div>

    <script>
        // communication stuff
        //protokol: command?arg1=value1&arg2=value2...
        // var ws = new WebSocket('ws://' + window.location.hostname + ':81');

        // ws.onopen = function() {
        //     console.log("Websocket connected");
        // };
        // ws.onmessage = function (evt) {
        //     const command = extractCommand(evt.data);
        //     const { argNames, args } = extractArgs(evt.data);
        //     console.log("Command: " + command);
        //     console.log("ArgNames: " + argNames);
        //     console.log("Args: " + args);
        // }

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
            minStepInput.value = 0;
            maxStepInput.value = 10;
            updateStepSliderLimits();
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
            console.log(url);
            // ws.send(url);
        }
        
    </script>

</body>
</html>



)rawliteral";

}
