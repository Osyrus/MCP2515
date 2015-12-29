/*  
  MCP2515.h - An Arduino library for interfacing with the MCP2515
  Standalone CAN Controller.
  Created by Myles R. Clark, 29th December 2015.

  Notes:

  --Choice of interrupt pin:
      *Board*                             *Valid Pins*
      Uno, Nano, Mini, other 328-based    2, 3
      Mega, Mega2560, MegaADK             2, 3, 18, 19, 20, 21
      Micro, Leonardo, other 32u4-based   0, 1, 2, 3, 7
      Zero                                all digital pins, except 4
      Due                                 all digital pins
  --On the receiver callback function:
     -The receiver function will be called during update() if there are any
      new received packets.
*/

#ifndef MCP2515_h
#define MCP2515_h

#include "SPI.h"

// Define the SPI instruction registers, see pg. 66 of the datasheet
#define RESET B11000000
#define READ B00000011
#define WRITE B00000010
#define READ_STATUS B10100000
#define RX_STATUS B10110000
#define BIT_MODIFY B00000101

// Packets buffer length
#define MAX_PACKETS_TX 10
#define MAX_PACKETS_RX 10

// This is the packet that is used to facilitate sending and receiving data
// on the CAN bus. Using packets allows for queuing.
struct packet {
  uint8_t id;
  uint8_t *data;
  uint8_t length;
  bool newPacket;
};

typedef void (* receiver)(uint16_t senderId, uint8_t *data, uint8_t length);

class MCP2515
{
  public:
    MCP2515(int SSPin = 10, uint32_t SPIClockSpeed = 1000000);

    void setSPICommSpeed(uint32_t clockSpeed);
    
    void setReceiverCallback(receiver r);

    bool send(uint16_t receiverId, uint8_t data[], uint8_t length);

    void update();

    bool writeToRegister(uint8_t reg, uint8_t data);
    uint8_t readFromRegister(uint8_t reg);

    uint8_t getReadStatus();
    uint8_t getRXStatus();
  private:
    int _interruptPin;

    uint8_t readShortCommand(uint8_t command);

    uint8_t data[8];
    packet _sendPackets[MAX_PACKETS_TX];
    packet _receivedPackets[MAX_PACKETS_RX];

    receiver _receiver;

    SPISettings _settings;
    int _SSPin;
};

#endif