#include "Mem24CSM01.h"

Mem24CSM01 memory(false, false);

void setup() {
    Serial.begin(9600);
    memory.begin();

    // Disabling Software WP (in case it was active)
    memory.disableSoftwareWriteProtect();
    
    MEMORYRESULT opResult;
    uint8_t write_byte = 80;
    opResult = memory.write(18, write_byte); // We write at adresse 18

    if(opResult==MEMORYRESULT::OK) {
      Serial.println("Write OK");
    } else {
      Serial.println("Error writing byte");
    }

    uint8_t read_byte = 0;
    opResult = memory.read(18, read_byte);
    if(opResult==MEMORYRESULT::OK) {
      Serial.print("Read OK, got value ");
      Serial.println(read_byte);
    } else {
      Serial.println("Error Reading byte");
    }
}

void loop() {
    // Nothing here
}
