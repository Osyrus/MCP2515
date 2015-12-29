#include "Arduino.h"
#include "MCP2515.h"
#include "SPI.h"

MCP2515::MCP2515(int SSPin, uint32_t SPIClockSpeed) {
  _SSPin = SSPin;
  setSPICommSpeed(SPIClockSpeed);
  SPI.begin();
}

void MCP2515::setSPICommSpeed(uint32_t clockSpeed) {
  _settings = SPISettings(clockSpeed, MSBFIRST, SPI_MODE0);
}

void MCP2515::setReceiverCallback(receiver r) {
  _receiver = r;
}

bool MCP2515::send(uint16_t receiverId, uint8_t data[], uint8_t length) {
  for (int i = 0; i < MAX_PACKETS_TX; i++) {
    if (_sendPackets[i].newPacket == NULL || !_sendPackets[i].newPacket) {
      _sendPackets[i].id = receiverId;
      _sendPackets[i].data = data;
      _sendPackets[i].length = length;
      _sendPackets[i].newPacket = true;
    }

    return true;
  }

  return false;
}

void MCP2515::update() {
  
}

// See pg 67 of the datasheet
bool MCP2515::writeToRegister(uint8_t reg, uint8_t data) {
  SPI.beginTransaction(_settings);
  digitalWrite(_SSPin, LOW);
  SPI.transfer(WRITE);
  SPI.transfer(reg);
  SPI.transfer(data);
  digitalWrite(_SSPin, HIGH);
  SPI.endTransaction();

  return (data == readFromRegister(reg));
}

// See pg 67 of the datasheet
uint8_t MCP2515::readFromRegister(uint8_t reg) {
  SPI.beginTransaction(_settings);
  digitalWrite(_SSPin, LOW);
  SPI.transfer(READ);
  SPI.transfer(reg);
  uint8_t response = SPI.transfer(0x00);
  digitalWrite(_SSPin, HIGH);
  SPI.endTransaction();

  return response;
}

uint8_t MCP2515::getReadStatus() {
  return readShortCommand(READ_STATUS);
}

uint8_t MCP2515::getRXStatus() {
  return readShortCommand(RX_STATUS);
}

uint8_t MCP2515::readShortCommand(uint8_t command) {
  SPI.beginTransaction(_settings);
  digitalWrite(_SSPin, LOW);
  SPI.transfer(command);
  uint8_t response = SPI.transfer(0x00);
  digitalWrite(_SSPin, HIGH);
  SPI.endTransaction();

  return response;
}