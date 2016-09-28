//Communication Interface
#include "CommItf.h"
#include "utility/wifi_utils.h"
/*
	TODO: select the correct driver, depends on the target MCU. supported drivers are:
	SPI: SPI Driver
	SER: Serial Driver
*/

//TODO the following const must be setted elsewhere
//#define END_CMD 0xEE
#define END_CMD 0xEE
#define START_CMD 0xE0

unsigned long _startMillis;
unsigned long _timeout = 1000; //1 Second Serial Timeout

CHANNEL CommChannel;
MCU McuType;
int BaudRate;
int SlaveSelectPin;

CommItf::CommItf()
{
	//TODO: MCU selection by compiler flags

	//TEST
	 McuType = AVR328P;

	 switch(int(McuType)){
	 	case AVR328P:
	 		CommChannel = CH_SERIAL;
	 		BaudRate = 9600;
	 	break;
	 	case NRF52:
	 		CommChannel = CH_SPI;
	 		SlaveSelectPin = 31;	//Internal GPIO of the NRF52
	 	break;
	 	case SAMD21:	//TODO test these identical cases
	 	case STM32:
	 		CommChannel = CH_SERIAL;
	 		BaudRate = 115200;
	 	break;
	 }
}

bool CommItf::begin()
{
	 if(CommChannel == CH_SERIAL){
	 	Serial.begin(BaudRate);

		//NOTE: debug only, remove for production env.
		Serial1.begin(BaudRate);

		while(!Serial);
		return true;
	 }
	 else if(CommChannel == CH_SPI){
		//TODO:
		//SPISlave.begin();
		//return true;
	}
	return false;
}

int CommItf::read(tMsgPacket *_pckt)
{
	if(CommChannel == CH_SERIAL){
		if(Serial.available()){
			return createPacketFromSerial(_pckt);
		}else{
			return -1;
		}
	}
	 else if(CommChannel == CH_SPI){
		return createPacketFromSPI(_pckt);
	}
}

int CommItf::createPacketFromSerial(tMsgPacket *_pckt){

		//TODO read message packet from serial
		//String raw_pckt = Serial.readStringUntil(END_CMD);
		String raw_pckt = readStringUntil(END_CMD);

		//for(int x=0;x<raw_pckt.length();x++)
		//	Serial1.println(raw_pckt[x],HEX);

		int idx = 0;
		unsigned char tmp = raw_pckt[idx];

		//Start Command
		if(tmp != START_CMD){
			//Error
			return -1;
		}
		_pckt->cmd = tmp;
		//Serial.write(_pckt->cmd);
		//The command
		tmp = raw_pckt[++idx];
		_pckt->tcmd = tmp;
		//Serial.write(_pckt->tcmd);
		//The number of parameters for the command
		tmp = raw_pckt[++idx];
		_pckt->nParam = tmp;
		//Serial.write(_pckt->nParam);
		//Get each parameter
		for(int a=0; a<(int)_pckt->nParam; a++){

			//Length of the parameter
			tmp = raw_pckt[++idx];
			_pckt->params[a].paramLen = tmp;
			//Serial.write(_pckt->params[a].paramLen);
			_pckt->params[a].param = (char*)malloc(_pckt->params[a].paramLen);
			//Value of the parameter
			for(int b=0; b<(int)_pckt->params[a].paramLen; b++){
				tmp = raw_pckt[++idx];
				//_pckt->params[a].param += (char)tmp; // _pckt->params[a].param type of String
				_pckt->params[a].param[b] = (char)tmp;	// _pckt->params[a].param type of char* []
				//Serial.write(_pckt->params[a].param[b]);
			}
		}
		//Serial.write(0xEE);

		//OK
		return 0;

}

int CommItf::createPacketFromSPI(tMsgPacket *_pckt){
	//TODO parse the message and create the packet
	return -1;
}

void CommItf::write(tMsgPacket *_pckt)
{
	if(CommChannel == CH_SERIAL){
		Serial.write(_pckt->cmd);
		Serial.write(_pckt->tcmd);
		Serial.write(_pckt->nParam);
		for(int i=0; i<(int)_pckt->nParam; i++){
			Serial.write(_pckt->params[i].paramLen);
			for(int j=0; j< (int)_pckt->params[i].paramLen; j++)
				Serial.write( _pckt->params[i].param[j]);
		}
		Serial.write(END_CMD);
	}
	else if(CommChannel == CH_SPI){
		//TODO
	}
}

void CommItf::end()
{
	if(CommChannel == CH_SERIAL){
		Serial.end();
	}
	else if(CommChannel == CH_SPI){
		//TODO
		//SPISlave.end()
	 }
}

//Private Methods/Functions
int CommItf::timedRead()
{
  int c;
  _startMillis = millis();
  do {
    c = Serial.read();
    if (c >= 0) return c;
  } while(millis() - _startMillis < _timeout);
  return -1;     // -1 indicates timeout
}

String CommItf::readStringUntil(char terminator)
{
  String ret;
  int c = timedRead();

  while (c >= 0 && (char)c != terminator)
  {
    ret += (char)c;
    c = timedRead();
  }
  return ret;
}

CommItf CommunicationInterface;
