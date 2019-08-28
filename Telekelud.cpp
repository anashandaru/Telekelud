#include "Arduino.h"
#include "SPI.h"
#include "LoRa.h"
#include "Adafruit_SleepyDog.h"
#include "Telekelud.h"

Telekelud::Telekelud(byte localAddress, byte destinationAddress, long interval){
	_localAddress = localAddress;
	_destinationAddress = destinationAddress;
	_interval = interval;

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

  _batt.sender = analogRead(VBATPIN);

  DEBUG_PRINTLN("LoRa init succeeded.");
}

void Telekelud::configure(){
  LoRa.setSpreadingFactor(SF);
  LoRa.setTxPower(TXP, PA_OUTPUT_PA_BOOST_PIN);
}

void Telekelud::setPacket(datpac packet){
	_packet = packet;
}

void Telekelud::writeFloat(const float& value){
  LoRa.write((byte *) &value, sizeof(value));
}

void Telekelud::readFloat(float& value){
  byte * p = (byte*) &value;
  for (int i = 0; i < 4; ++i)
  {
    *p++ = LoRa.read();
  }
}

void Telekelud::sendMessage(){

  LoRa.beginPacket();                   // start packet
  LoRa.write(_destinationAddress);      // add destination address
  LoRa.write(_localAddress);            // add sender address
  LoRa.write(_msgCount);                // add message ID

  writeFloat(_packet.gas);
  writeFloat(_packet.tm1);
  writeFloat(_packet.tm2);
  writeFloat(_packet.ph);
  writeFloat(_packet.tds);
  writeFloat(_packet.dis);
  writeFloat(_batt.sender);
  writeFloat(_batt.repeater);

  LoRa.endPacket();                     // finish packet and send it

  DEBUG_PRINT("Sending :");
  DEBUG_PRINT(_packet.gas); DEBUG_PRINT(",");
  DEBUG_PRINT(_packet.tm1); DEBUG_PRINT(",");
  DEBUG_PRINT(_packet.tm2); DEBUG_PRINT(",");
  DEBUG_PRINT(_packet.ph); DEBUG_PRINT(",");
  DEBUG_PRINT(_packet.tds); DEBUG_PRINT(",");
  DEBUG_PRINT(_packet.dis); DEBUG_PRINT(",");
  DEBUG_PRINTLN("ends");
  
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

  // if the recipient isn't this device or broadcast,
  if (recipient != _localAddress && recipient != 0xFF) {
    DEBUG_PRINTLN("This message is not for me.");
    return 0;                             // skip rest of function
  }

  readFloat(_packet.gas);
  readFloat(_packet.tm1);
  readFloat(_packet.tm2);
  readFloat(_packet.ph);
  readFloat(_packet.tds);
  readFloat(_packet.dis);
  readFloat(_batt.sender);
  readFloat(_batt.repeater);

  _msgCount = incomingMsgId;

  DEBUG_PRINT("Received :");
  DEBUG_PRINT(_packet.gas); DEBUG_PRINT(",");
  DEBUG_PRINT(_packet.tm1); DEBUG_PRINT(",");
  DEBUG_PRINT(_packet.tm2); DEBUG_PRINT(",");
  DEBUG_PRINT(_packet.ph); DEBUG_PRINT(",");
  DEBUG_PRINT(_packet.tds); DEBUG_PRINT(",");
  DEBUG_PRINT(_packet.dis); DEBUG_PRINT(",");
  DEBUG_PRINTLN("ends");

  return 1;
}

void Telekelud::repeaterService(){
	if(!listen()) return;
  _batt.repeater = analogRead(VBATPIN);
	sendMessage();
	listenMode();
}

void Telekelud::senderServicePS(){
	sleep();
	_batt.sender = analogRead(VBATPIN);
	sendMessage();
}

void Telekelud::repeaterServicePS(){
	if(!listen()) return;
  _batt.repeater = analogRead(VBATPIN);
	sendMessage();
	sleep();
	listenMode();
}

int Telekelud::bat2percent(int bat){
  // converts adc reading to battery percentage (1 - 100)
  // 3.2v(0%) to 4.2v(100%)
  // 551 to 651, 651 - 551 = 100
  int result = (bat-551);
  if(result>100) result = 100;
  return result;
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

int Telekelud::getRSSI(){
	return LoRa.packetRssi();
}

float Telekelud::getSNR(){
	return LoRa.packetSnr();
}