#include "Mem24CSM01.h"

Mem24CSM01 memory(false, false);

void explainConfig(uint16_t config) {
  Serial.print("Config is ");
  Serial.println(config, BIN);
  bool correctionWasRequired = (config >> 15) & 1;
  bool ewpActive = (config >> 9) & 1;
  bool locked = (config >> 7) & 1;

  if (correctionWasRequired) Serial.println("Correction Error Code was required on last read operation");
  if (locked) Serial.println("Configuration is locked");
  if (ewpActive) {
    Serial.println("Enhanced Write Protection active");
    for (int i = 0; i < 8; ++i) {
      if ((config >> i) & 1) {
        Serial.print("Memory zone ");
        Serial.print(i);
        Serial.println(" protected");
      }
    }
  } else {
    Serial.println("Enhanced Write Protection inactive");
  }
}

void setup() {
    Serial.begin(9600)
    memory.begin();

    // Reading the EEPROM Configuration and displaying it on Serial monitor
    uint16_t config = memory.getConfiguration();
    explainConfig(config);

    // Enabling Software Write Protect & protecting zone 0
    memory.enableSoftwareWriteProtect();
    memory.setWriteProtectionZone(0);
    config = memory.getConfiguration();
    explainConfig(config);

    // Disabling Software WP
    memory.disableSoftwareWriteProtect();
    
}

void loop() {
    // Nothing here
}