#ifndef TELEKELUD_H
  
  #define DEBUG
  #define BAND    915E6
  #define SF      12
  #define TXP     17

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

class Telekelud
{
public:
  Telekelud(byte localAddress, byte destinationAddress, long interval);
  void start();
  void configure();
  void setTemp(int value);
  void setPH(int value);
  void sendMessage();
  void sendMessageEvery(long interval);
  void listenMode();
  bool listen();
  void senderService();
  void repeaterService();
  void senderServicePS();
  void repeaterServicePS();
  int bat2percent(int bat);
  void setLed(bool on);
  void sleep(int duration); // duration in seconds
  int getTemp();
  int getPH();
  int getVbatSender();
  int getVbatRepeater();
  int getRSSI();
  float getSNR();

private:
  int _temp;
  int _ph;
  int _vbatSender;
  int _vbatRepeater;
  byte _msgCount;
  byte _localAddress;
  byte _destinationAddress;
  long _lastSendTime;
  long _interval;
};

#endif