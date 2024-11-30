#include <FS.h>
#include <SPIFFS.h>

void setup() {
    Serial.begin(115200);

    // Initialize SPIFFS
    if (!SPIFFS.begin(true)) {
        Serial.println("SPIFFS initialization failed!");
        return;
    }

    Serial.println("Files in SPIFFS:");
    // Create a Dir object to list files
    File dir = SPIFFS.open("/");
    File file = dir.openNextFile();
    
    // List files in SPIFFS
    while (file) {
        Serial.print(" - ");
        Serial.println(file.name());
        file = dir.openNextFile();
    }
}

void loop() {
    // Your main code
}
