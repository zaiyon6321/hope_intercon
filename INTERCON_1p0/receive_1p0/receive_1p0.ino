#include "sbus.h"
#include "Arduino.h"
#include "e32433t30d.h"

#define DEBUG

#define SBUS_TX   17
#define SBUS_RX   16

#define START_BYTE 255
#define STOP_BYTE  0

bfs::SbusRx sbus_rx(&Serial2, SBUS_RX, SBUS_TX, true, false);
/* SBUS object, writing SBUS */
bfs::SbusTx sbus_tx(&Serial2, SBUS_RX, SBUS_TX, true, false);

bfs::SbusData radioData;


// Define pins for UART1
#define E32_TX 26  // ESP32 TX → E32 RX
#define E32_RX 27  // ESP32 RX ← E32 TX
#define M0_PIN 2
#define M1_PIN 4
#define AUX_PIN 5

#define RADIO_CHANNEL 16

HardwareSerial E32_UART(1); // create hard ware serial object for E32 module
E32 E32_obj(&E32_UART, _115200, _9P6 , E32_RX, E32_TX, M0_PIN, M1_PIN, AUX_PIN); //e32 module object


struct {
  union{
    uint8_t byteData[RADIO_CHANNEL * 2]; //[H, L, H, L, H, L]
    uint16_t intData[RADIO_CHANNEL];     //[HL, HL, HL]
  };
}frame;

uint8_t e32_readState = 0, dataCount = 0, dataSize = 0;//state machine variable for e32 reading frame
uint8_t temp[RADIO_CHANNEL * 2 + 2];

void setup() {
  Serial.begin(115200);

  //initialization for subs 
  sbus_rx.Begin();
  sbus_tx.Begin();

  //initialization for e32 module
  E32_obj.init();
  uint8_t param[6];
  E32_obj.getParam(param);
  E32_obj.showParam(param);

}

void loop() {

  while(E32_obj.available()) {
    uint8_t c = E32_obj.getData(); // read one byte
    //Serial.println(c);
    switch(e32_readState){
      case 0:
            dataCount = 0;
            if(c==255){
              e32_readState = 1;
            }
            else
            e32_readState = 0;
      break;

      case 1:
            
            if(c==255){
              e32_readState = 2;
            }
            else
              e32_readState = 0;
      break;

      case 2:
            dataSize = c;
            e32_readState = 3;
      break;

      case 3:
            temp[dataCount] = c;
            //Serial.println(dataSize);
            if(dataCount==((dataSize*2)) && c == 0)
            {
              e32_readState = 0;
              for(uint8_t i=0; i<dataSize; i++){
                frame.intData[i] = (uint16_t)((temp[i*2]) | temp[(i*2)+1]<<8);
                Serial.print(frame.intData[i]);
                Serial.print(" ");
              }
              Serial.println();
            }            
            else if(dataCount >= ((dataSize*2)))
            {
              e32_readState = 0;
            }
            dataCount++;
      break;
    }

    if(dataCount >= (dataSize*2))
    {
      break;
    }
  }
  /*for(uint8_t i=0; i<RADIO_CHANNEL; i++){
  Serial.print(frame.intData[i]);
  Serial.print("      ");
  }
  Serial.println();*/

}
















