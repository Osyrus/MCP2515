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
  // First check to see if there are any new messages received by checking
  // the MCP2515's status.
  uint8_t status = getReadStatus();

  bool reg[] = {false, false};
  uint8_t command[] = {0x61, 0x71};
  TwoByte idData;

  // First check read register 0
  if ((status & B00000001) == B00000001) {
    reg[0] = true;
  }
  // Then check read register 1
  if ((status & B00000010) == B00000010) {
    reg[1] = true;
  }

  // Now read the data from the registers, if required
  for (int i = 0; i < 2; i++) {
    if (reg[i]) {
      // First, we start the SPI up and pull CS low
      SPI.beginTransaction(_settings);
      digitalWrite(_SSPin, LOW);
      // The MCP2515 wants the command next, so we give it that
      SPI.transfer(command[i]);
      // The next two bytes for a single int with the senders id info
      idData.b[0] = SPI.transfer(0x00);
      idData.b[1] = SPI.transfer(0x00);
      // The next two bytes are only used with extended frames, as I am
      // not using that, they can be dumped.
      SPI.transfer(0x00);
      SPI.transfer(0x00);
      // The next byte contains info about the number of data bytes in
      // this frame, which will inform the loop reading the data. This
      // byte is shared with other data, so we also need to mask that out.
      uint8_t length = SPI.transfer(0x00);
      length = length & B00001111;
      // Now the next bytes (of which the number is now stored in length),
      // can be read out into an array.
      uint8_t data[length];
      for (int i = 0; i < length; i++) {
        data[i] = SPI.transfer(0x00);
      }
      // Now we are done with SPI
      digitalWrite(_SSPin, HIGH);
      SPI.endTransaction();

      // Now we can clean up the id data, which has 5 low bits that aren't
      // useful here.
      uint16_t sendersId = idData.value;
      sendersId = sendersId >> 5;
      // Now we store the data into this registers packet to process later
      _readPackets[i].id = sendersId;
      _readPackets[i].data = data;
      _readPackets[i].length = length;
      _readPackets[i].newPacket = true;
    }
  }
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

// See pg 69 for the meaning of these registers
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