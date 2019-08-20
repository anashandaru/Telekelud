#ifndef TELEKELUD_H
  
  #define DEBUG
  #define BAND    915E6
  #define SF      12
  #define TXP     20

// LoRa Module SPI pin definition
  #define SCK     15
  #define MISO    14
  #define MOSI    16
  #define SS      8
  #define RST     4
  #define DI0     7
  #define VBATPIN A9


// set to DEBUG to activate Serial
#ifdef DEBUG
  #define DEBUG_WAIT           !Serial
  #define DEBUG_PRINT(x)       Serial.print (x)
  #define DEBUG_PRINT2(x,y)    Serial.print (x,y)
  #define DEBUG_PRINTDEC(x)    Serial.print (x, DEC)
  #define DEBUG_PRINTLN(x)     Serial.println (x)
  #define DEBUG_PRINTLN2(x,y)  Serial.println (x,y)
  #define DEBUG_START(x)       Serial.begin(x)
  #define DEBUG_ATTACH()       USBDevice.attach();
#else
  #define DEBUG_WAIT           false
  #define DEBUG_PRINT(x)
  #define DEBUG_PRINT2(x,y)
  #define DEBUG_PRINTDEC(x)
  #define DEBUG_PRINTLN(x)
  #define DEBUG_PRINTLN2(x,y)
  #define DEBUG_START(x)
  #define DEBUG_ATTACH()
#endif

#include "Arduino.h"
#include "SPI.h"
#include "LoRa.h"
#include "Adafruit_SleepyDog.h"

struct datpac
{
  float gas;
  float tm1;
  float tm2;
  float ph;
  float tds;
  float dis;
};

struct batpac
{
  float sender;
  float repeater;
};

class Telekelud
{
public:
  Telekelud(byte localAddress, byte destinationAddress, long interval);
  void start();
  void configure();
  void setPacket(datpac packet);
  void sendMessage();
  void sendMessageEvery(long interval);
  void listenMode();
  bool listen();
  void repeaterService();
  void senderServicePS();
  void repeaterServicePS();
  void setLed(bool on);
  void sleep(); // duration in seconds
  void writeFloat(const float& value);
  void readFloat(float& value);
  int getRSSI();
  float getSNR();

private:
  datpac _packet;
  batpac _batt;
  byte _msgCount;
  byte _localAddress;
  byte _destinationAddress;
  long _lastSendTime;
  long _interval;
};

#endif