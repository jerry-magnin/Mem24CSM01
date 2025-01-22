#include "Mem24CSM01.h"

Mem24CSM01 memory(false, false);

void setup() {
    Serial.begin(9600);
    memory.begin();

    // Disabling Software WP (in case it was active)
    memory.disableSoftwareWriteProtect();
    
    MEMORYRESULT opResult;
    uint8_t byteArrayWrite[3] = {20, 40, 80};
    uint8_t byteArrayRead[3] = {0};
    opResult = memory.write(18, byteArrayWrite, sizeof(byteArrayWrite)); // We write at adresse 18
    if(opResult==MEMORYRESULT::OK) {
      Serial.println("Write OK");
    } else {
      Serial.println("Error writing bytes");
    }

    opResult = memory.read(18, byteArrayRead, sizeof(byteArrayRead));
    if(opResult==MEMORYRESULT::OK) {
      Serial.println("Read OK, got values:");
      for(int i=0; i<3; ++i) {
        Serial.print("Byte ");
        Serial.print(i);
        Serial.print(" = ");
        Serial.println(byteArrayRead[i]);
      }
    } else {
      Serial.println("Error Reading bytes");
    }
}

void loop() {
    // Nothing here
}
