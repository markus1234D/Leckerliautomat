#ifndef DATAWORKER_H
#define DATAWORKER_H

#include <Arduino.h>
#include <FS.h>
#include <LittleFS.h>

#define FORMAT_LITTLEFS_IF_FAILED true


class DataWorker {
public:
    DataWorker();
    String getFileContent(const char * path);

private:

private:
    // String readFileToString(fs::FS &fs, const char * path);

};

DataWorker::DataWorker() {

    if(!LittleFS.begin(FORMAT_LITTLEFS_IF_FAILED)){
        Serial.println("LittleFS Mount Failed");
        return;
    }
    Serial.println("LittleFS Mount Success");
}


String DataWorker::getFileContent(const char * path) {
    Serial.printf("Reading file: %s\n", path);
    String content;
    File file = LittleFS.open(path);
    if (!file) {
        Serial.println("Failed to open file for reading");
        return "failed";
    }
    while (file.available()) {
        content += String((char)file.read());
    }
    file.close();
    return content;

    // return R"rawliteral(
    //     <html>
    //         <head>
    //             <title>File Content</title>
    //         </head>
    //         <body>
    //             <h1>File Content</h1>
    //         </body>
    //     </html>
    // )rawliteral";
}






#endif // DATAWORKER_H