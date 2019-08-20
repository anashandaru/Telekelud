#include "Arduino.h"
#include "SPI.h"
#include "LoRa.h"
#include "Adafruit_SleepyDog.h"
#include "Telekelud.h"

Telekelud::Telekelud(byte localAddress, byte destinationAddress, long interval){
	_localAddress = localAddress;
	_destinationAddress = destinationAddress;
	_interval = interval;

	_temp     = 0;
	_ph       = 0;
	_msgCount = 0;

}

void Telekelud::start(){
  DEBUG_START(9600);
  while (DEBUG_WAIT)

  pinMode(LED_BUILTIN, OUTPUT);
  setLed(1);

  LoRa.setPins(SS,RST,DI0);
  if (!LoRa.begin(BAND)) {
    DEBUG_PRINTLN("Starting LoRa failed!");
    while (1);
  }

  DEBUG_PRINTLN("LoRa init succeeded.");
}

void Telekelud::configure(){
  LoRa.setSpreadingFactor(SF);
  LoRa.setTxPower(TXP, PA_OUTPUT_PA_BOOST_PIN);
}

void Telekelud::setTemp(int value){
	_temp = value;
}

void Telekelud::setPH(int value){
	_ph = value;
}

void Telekelud::sendMessage(){
  byte buff[8];
  buff[0] = (_temp >> 8) & 0xFF;
  buff[1] = _temp & 0xFF;
  buff[2] = (_ph >> 8) & 0xFF;
  buff[3] = _ph & 0xFF;
  buff[4] = (_vbatSender >> 8) & 0xFF;
  buff[5] = _vbatSender & 0xFF;
  buff[6] = (_vbatRepeater >> 8) & 0xFF;
  buff[7] = _vbatRepeater & 0xFF;

  LoRa.beginPacket();                   // start packet
  LoRa.write(_destinationAddress);      // add destination address
  LoRa.write(_localAddress);            // add sender address
  LoRa.write(_msgCount);                // add message ID
  for (int i = 0; i < 8; ++i)           // add payload data
  {
  	LoRa.write(buff[i]);
  }
  LoRa.endPacket();                     // finish packet and send it

  DEBUG_PRINT("Sending ");
  DEBUG_PRINTLN(" T: "      +String(_temp)+
              " Tj: "      +String(_ph)+
              " Sbat: "  +String(_vbatSender)+
  	          " Rbat: "  +String(_vbatRepeater)+
              " msgID: " +String(_msgCount));
  DEBUG_PRINTLN();
  
  _msgCount++;                    // increment message ID
}

void Telekelud::sendMessageEvery(long interval){
  if (millis() - _lastSendTime > _interval) {
    sendMessage();
    _lastSendTime = millis();            // timestamp the message
  }
}

void Telekelud::listenMode(){
	LoRa.receive();
}

bool Telekelud::listen(){
  int packetSize = LoRa.parsePacket();
  
  if(packetSize == 0) return 0;

  // read packet header bytes
  int recipient = LoRa.read();          // recipient address
  byte sender = LoRa.read();            // sender address
  byte incomingMsgId = LoRa.read();     // incoming msg ID

  int dataCandidate[4];                 // payload of packet

  for (int i = 0; i < 4; ++i){
  	dataCandidate[i]   = LoRa.read();
  	dataCandidate[i] <<= 8;
  	dataCandidate[i]  |= LoRa.read();
  }

  // if the recipient isn't this device or broadcast,
  if (recipient != _localAddress && recipient != 0xFF) {
    DEBUG_PRINTLN("This message is not for me.");
    return 0;                             // skip rest of function
  }

  _temp = dataCandidate[0];
  _ph = dataCandidate[1];
  _vbatSender = dataCandidate[2];
  _vbatRepeater = dataCandidate[3];
  _msgCount = incomingMsgId;

  DEBUG_PRINT("Received ");
  DEBUG_PRINTLN(" from: 0x" +String(sender, HEX)+
                " to: 0x"   +String(recipient,HEX)+
                " id:"      +String(incomingMsgId)+
                " T"        +String(_temp)+" Sbat"+String(_vbatSender)+" Rbat"+String(_vbatRepeater)+
                " RSSI:"    +String(LoRa.packetRssi())+
                " snr:"     +String(LoRa.packetSnr())
                );
  return 1;
}

void Telekelud::senderService(){
  _vbatSender = analogRead(VBATPIN);
  sendMessageEvery(_interval);
}

void Telekelud::repeaterService(){
	if(!listen()) return;
    _vbatRepeater = analogRead(VBATPIN);
	sendMessage();
	listenMode();
}

void Telekelud::senderServicePS(){
	sleep();
	_vbatSender = analogRead(VBATPIN);
	sendMessage();
}

void Telekelud::repeaterServicePS(){
	if(!listen()) return;
    _vbatRepeater = analogRead(VBATPIN);
	sendMessage();
	sleep();
	listenMode();
}

void Telekelud::setLed(bool on){
  digitalWrite(LED_BUILTIN, on);
}

void Telekelud::sleep(){
  int count = _interval/8000;
  DEBUG_PRINTLN("sleep for "+String(count)+" * 8000");
  setLed(0);

  LoRa.sleep();
  for (int i = 0; i < count; ++i)
  {
      Watchdog.sleep(8000);
      //delay(8000);
  }

  setLed(1);
  DEBUG_ATTACH();
}

int Telekelud::getTemp(){
	return _temp;
}

int Telekelud::getPH(){
	return _ph;
}

int Telekelud::getVbatSender(){
	return _vbatSender;
}

int Telekelud::getVbatRepeater(){
	return _vbatRepeater;
}

int Telekelud::getRSSI(){
	return LoRa.packetRssi();
}

float Telekelud::getSNR(){
	return LoRa.packetSnr();
}

void Telekelud::setSenderBatt(int value){
	_vbatSender = value;
}