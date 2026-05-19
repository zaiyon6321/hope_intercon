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
#define AUX_PIN 34

#define RADIO_CHANNEL 3



HardwareSerial E32_UART(1); // create hard ware serial object for E32 module
E32 E32_obj(&E32_UART, _115200, _9P6 , E32_RX, E32_TX, M0_PIN, M1_PIN, AUX_PIN); //e32 module object


struct {
  union{
    uint8_t byteData[RADIO_CHANNEL * 2]; //[L, H, L, H, L, H]
    uint16_t intData[RADIO_CHANNEL];     //[LH, LH, LH]
  };
}frame;
char radioFrame[50];

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

  if (sbus_rx.Read()) 
  {
      radioData = sbus_rx.data();

      #ifdef DEBUG
        Serial.print("Radio ");
        for(uint8_t i=0; i<RADIO_CHANNEL; i++){
          Serial.print(radioData.ch[i]);
          Serial.print("  ");
        }
        Serial.println();
      #endif


      for(uint8_t i=0; i<RADIO_CHANNEL; i++){
        frame.intData[i] = radioData.ch[i];
      }


      radioFrame[0]=0xDD;
      radioFrame[1]=0xCC;
      radioFrame[2]=0x0B;
      radioFrame[3]=START_BYTE;
      radioFrame[4]=START_BYTE;
      radioFrame[5]=RADIO_CHANNEL;

      for(uint8_t i=0; i<(RADIO_CHANNEL*2); i++){
        radioFrame[6+i] = frame.byteData[i];
      }
      radioFrame[(6+(RADIO_CHANNEL*2))]=STOP_BYTE;

      E32_obj.sendData((uint8_t *)radioFrame, (7+(RADIO_CHANNEL*2)));

  }
  delay(70);


}