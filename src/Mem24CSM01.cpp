#include "Mem24CSM01.h"


uint8_t zoneProtection(bool zone7=0, bool zone6=0, bool zone5=0, bool zone4=0, bool zone3=0, bool zone2=0, bool zone1=0, bool zone0=0) {
  uint8_t protectionPattern = 0;
  protectionPattern |= (zone7<<7) | (zone6<<6) | (zone5<<5) | (zone4<<4) | (zone3<<3) | (zone2<<2) | (zone1<<1) | zone0;
  return(protectionPattern);
}

Mem24CSM01::Mem24CSM01(uint8_t memoryRegister) {
  m_memory_register = memoryRegister;
  m_configuration_register = m_memory_register & ~(1<<4);
}

Mem24CSM01::Mem24CSM01(bool A1, bool A2) {
  m_memory_register = BASE_MEMREG_ADDR | (A2<<3) | (A1<<2);
  m_configuration_register = BASE_CFGREG_ADDR | (A2<<3) | (A1<<2); 
}

void Mem24CSM01::begin() {
  Wire.begin();
}

uint16_t Mem24CSM01::getConfiguration() {
  uint16_t result;
  uint8_t low, high;
  Wire.beginTransmission(m_configuration_register);
  Wire.write(CFGREG_WRD_ADDRH);
  Wire.write(CFGREG_WRD_ADDRL);
  Wire.endTransmission(false);
  //Wire.requestFrom(0b1011000, 2, true);
  Wire.requestFrom(m_configuration_register, 2, true);
  high = Wire.read();
  low = Wire.read();
  result = (high<<8) | low;
  m_configuration.zoneProtection = low;
  m_configuration.configLocked = high & 1;
  m_configuration.softwareWriteProtect = (high>>1) & 1;
  m_configuration.errorCorrectionState = (high>>7) & 1;
  return(result);
}

bool Mem24CSM01::updateConfiguration(uint8_t confirmLock=0x66) {
  // Preparing the config bytes
  uint8_t cfgHighByte = 0 | (m_configuration.softwareWriteProtect<<1) | m_configuration.configLocked;
  uint8_t cfgLowByte = m_configuration.zoneProtection;
  Serial.println(cfgLowByte, BIN);
  Wire.beginTransmission(m_configuration_register);
  Wire.write(CFGREG_WRD_ADDRH);
  Wire.write(CFGREG_WRD_ADDRL);
  Wire.write(cfgHighByte);
  Wire.write(cfgLowByte);
  Wire.write(confirmLock);
  Wire.endTransmission();
}

bool Mem24CSM01::enableSoftwareWriteProtect() {
  m_configuration.softwareWriteProtect = 1;
  updateConfiguration();
}
bool Mem24CSM01::disableSoftwareWriteProtect() {
  m_configuration.softwareWriteProtect = 0;
  updateConfiguration();
}

bool Mem24CSM01::setWriteProtectionZone(uint8_t zone) {
  if(zone >=0 && zone <=7) {
    m_configuration.zoneProtection = bitSet(m_configuration.zoneProtection, zone);
    updateConfiguration();
    return(true);
  } else {
    return(false);
  }
}

bool Mem24CSM01::writeProtection(uint8_t zones) {
  m_configuration.zoneProtection = zones;
  updateConfiguration();
}

bool Mem24CSM01::removeWriteProtectionZone(uint8_t zone) {
  if(zone >= 0 && zone <= 7) {
    m_configuration.zoneProtection = bitClear(m_configuration.zoneProtection, zone);
    updateConfiguration();
    return(true);
  } else {
    return(false);
  }
}

MemoryPointer Mem24CSM01::setMemoryAddress(uint32_t address) {
  MemoryPointer memoryAddress;
  uint8_t highAddrBit = (address>>17) & 1;
  memoryAddress.deviceMemoryAddress = m_memory_register | (highAddrBit<<1);
  memoryAddress.memoryMSB = (address>>8) & 0xFF;
  memoryAddress.memoryLSB = address & 0xFF;
  return(memoryAddress);
}

MEMORYRESULT Mem24CSM01::processTransmissionResult(int transmissionResult) {
  switch (transmissionResult) {
  case 0:
    return(MEMORYRESULT::OK);
    break;
  case 2:
    return(MEMORYRESULT::ADDRESS_ERROR);
    break;
  case 3:
    return(MEMORYRESULT::DATA_ERROR);
    break;
  case 5:
    return(MEMORYRESULT::TIMEOUT);
    break;
  default:
    return(MEMORYRESULT::GENERIC_ERROR);
    break;
  }
}

uint8_t Mem24CSM01::addressMemoryPointer(uint32_t address) {
  MemoryPointer memoryAddress = setMemoryAddress(address);
  Wire.beginTransmission(memoryAddress.deviceMemoryAddress);
  Wire.write(memoryAddress.memoryMSB);
  Wire.write(memoryAddress.memoryLSB);
  return(memoryAddress.deviceMemoryAddress);
}

uint8_t Mem24CSM01::read() {  // Read at current adress pointer
  Wire.beginTransmission(m_memory_register);
  Wire.endTransmission();
  Wire.requestFrom(m_memory_register, 1);
  return(Wire.read());
}

uint8_t Mem24CSM01::read(uint32_t address) {// Random read {
  uint8_t result = 0;
  read(address, &result, 1);
  return(result);
} 

MEMORYRESULT Mem24CSM01::read(uint32_t address, uint8_t* buffer, size_t bufferSize) {
  if(address > MAX_MEMORY_POINTER) { return(MEMORYRESULT::ADDR_TOO_LARGE); }
  if(address < 0) { return(MEMORYRESULT::NEGATIVE_ADDR); }
  if(bufferSize>256) {return(MEMORYRESULT::BUFFER_TOO_LARGE);}
  uint8_t deviceAddress = addressMemoryPointer(address);
  Wire.endTransmission();
  Wire.requestFrom(deviceAddress, bufferSize);
  size_t i=0;
  while(Wire.available()) {
    buffer[i] = Wire.read();
    Serial.println(buffer[i]);
    ++i;
  }
  return(MEMORYRESULT::OK);
}

MEMORYRESULT Mem24CSM01::write(uint32_t address, uint8_t singleByte) {
  return(write(address, &singleByte, 1));
}

MEMORYRESULT Mem24CSM01::write(uint32_t address, uint8_t* dataArray, size_t arraySize) {
  if(address > MAX_MEMORY_POINTER) { return(MEMORYRESULT::ADDR_TOO_LARGE); }
  if(address < 0) { return(MEMORYRESULT::NEGATIVE_ADDR); }
  if(arraySize>256) {return(MEMORYRESULT::BUFFER_TOO_LARGE);}
  if((address + arraySize - 1) > 0xFF) {return(MEMORYRESULT::NOT_ON_SINGLE_PAGE);}
  MemoryPointer memoryAddress = setMemoryAddress(address);
  Wire.beginTransmission(memoryAddress.deviceMemoryAddress);
  Wire.write(memoryAddress.memoryMSB);
  Wire.write(memoryAddress.memoryLSB);
  for(int i=0; i<arraySize; ++i) {
    Wire.write(dataArray[i]);
  }
  int transmissionResult = Wire.endTransmission();
  return(processTransmissionResult(transmissionResult));
}