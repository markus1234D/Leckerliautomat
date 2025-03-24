#include <Arduino.h>
#include "GuiWorker.h"

// put function declarations here:
GuiWorker guiWorker;

void setup() {
  Serial.begin(115200);
  Serial.println("Hello World!");
  guiWorker.init();
}

void loop() {
  guiWorker.handleGui();
  delay(20);
}
