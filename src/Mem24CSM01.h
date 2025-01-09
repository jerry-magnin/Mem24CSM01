#include <stdint.h>
#include "Arduino.h"
#include <Wire.h>

#define BASE_MEMREG_ADDR 0b1010<<3
#define BASE_CFGREG_ADDR 0b1011000

#define CFGREG_WRD_ADDRH 0b10001000
#define CFGREG_WRD_ADDRL 0b00000000

#define CFGREG_EWPM 9
#define CFGREG_LOCK 8

#define CONFIRM_LOCK 0x99
#define NO_LOCK 0x66

#define MEMZONE0 0
#define MEMZONE1 0x04000
#define MEMZONE2 0x08000
#define MEMZONE3 0x0C000
#define MEMZONE4 0x10000
#define MEMZONE5 0x14000
#define MEMZONE6 0x18000
#define MEMZONE7 0x1C000

#define MAX_MEMORY_POINTER 0x1FFFF

typedef enum {
  OK,
  ADDR_TOO_LARGE,
  BUFFER_TOO_LARGE,
  NEGATIVE_ADDR,
  NOT_ON_SINGLE_PAGE,
  ADDRESS_ERROR,
  DATA_ERROR,
  TIMEOUT,
  GENERIC_ERROR,
} MEMORYRESULT;

typedef struct {
  bool errorCorrectionState;
  bool softwareWriteProtect;
  bool configLocked;
  uint8_t zoneProtection;
} MemoryConfig;

typedef struct {
  uint8_t deviceMemoryAddress;
  uint8_t memoryMSB;
  uint8_t memoryLSB;
} MemoryPointer;

uint8_t zoneProtection(bool zone7=0, bool zone6=0, bool zone5=0, bool zone4=0, bool zone3=0, bool zone2=0, bool zone1=0, bool zone0=0);

class Mem24CSM01 {
  public:
    Mem24CSM01(uint8_t memoryRegister);
    Mem24CSM01(bool A1, bool A2);
    void begin();

    uint16_t getConfiguration();
    bool updateConfiguration(uint8_t confirmLock=0x66);
    bool enableSoftwareWriteProtect();
    bool disableSoftwareWriteProtect();
    bool setWriteProtectionZone(uint8_t zone);
    bool writeProtection(uint8_t zones);
    bool removeWriteProtectionZone(uint8_t zone);

    uint8_t read(); // Read at current adress pointer
    uint8_t read(uint32_t address); // Random read
    MEMORYRESULT read(uint32_t address, uint8_t* buffer, size_t size);
    MEMORYRESULT write(uint32_t address, uint8_t singleByte);
    MEMORYRESULT write(uint32_t address, uint8_t* dataArray, size_t arraySize);

  private:
    MemoryPointer setMemoryAddress(uint32_t address);
    MEMORYRESULT processTransmissionResult(int transmissionResult);
    uint8_t addressMemoryPointer(uint32_t address);
    uint8_t m_memory_register;
    uint8_t m_configuration_register;
    uint8_t m_security_register;
    MemoryConfig m_configuration;
};