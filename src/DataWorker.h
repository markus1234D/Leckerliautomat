#ifndef DATAWORKER_H
#define DATAWORKER_H

#include <Arduino.h>
#include <FS.h>
#include <LittleFS.h>
#include <stdarg.h>
#include "Debug.h"

#define FORMAT_LITTLEFS_IF_FAILED true


class DataWorker {
public:
    DataWorker();
    String getFileContent(const char * path);

private:
    void debugPrint(const char* format, ...);

    // String readFileToString(fs::FS &fs, const char * path);

};

void DataWorker::debugPrint(const char* format, ...) {
#ifdef DEBUG
    va_list args;
    va_start(args, format);
    writeDebugMessage("DataWorker", format, args);
    va_end(args);
#else
    (void)format;
#endif
}

DataWorker::DataWorker() {

    if(!LittleFS.begin(FORMAT_LITTLEFS_IF_FAILED)){
        debugPrint("LittleFS Mount Failed");
        return;
    }
    debugPrint("LittleFS Mount Success");
}


String DataWorker::getFileContent(const char * path) {
    debugPrint("Reading file: %s", path);
    String content;
    File file = LittleFS.open(path);
    if (!file) {
        debugPrint("Failed to open file for reading");
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