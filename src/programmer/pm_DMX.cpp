#include <Arduino.h>
#include "pm_DMX.h"


void pm_DMX::init()
    {
        if(!_dmx_init)
        {
            Serial1.setTxBufferSize(384);
            Serial1.begin(DMX_baud_rate, DMX_serial_config, DMX_revicePin, DMX_sendPin);
            _dmx_init=true;
        }
        
    }
uint8_t pm_DMX::read(int Channel)
    {
      if (!_dmx_init)init();
        
      if (Channel < 1)Channel = 1;
        
      if (Channel > dmxMaxChannel)Channel = dmxMaxChannel;
        
      return (dmxData[Channel]);
    }

void pm_DMX::write(int Channel, uint8_t value)
    {
      if (!_dmx_init)init();

      if (Channel < 1)Channel = 1;
        
      if (Channel > chanSize)Channel = chanSize;
        
      if (value < 0)value = 0;
        
      if (value > 255)value = 255;
        
      dmxData[Channel] = value;
    }
uint8_t Address[10] = {0x91, 0x20, 0x11, 0xAA, 0, 1, 0x55, 0, 1, 0};    // Address setting command
void pm_DMX::writeAddress(int address)
    {
        uint8_t check_sum=0;
        if(address<0 || address>511)address=0;
        Address[4] = (address>>8) & 0Xff;
        Address[7] = (address>>8) & 0Xff;
        Address[5] = address & 0Xff;
        Address[8] = address & 0Xff;
        for(int i=0;i<9;i++){
            check_sum+=Address[i];
        }
        Address[9]=check_sum;
        digitalWrite(DMX_sendPin, LOW);
        delay(2);
        digitalWrite(DMX_sendPin, HIGH);

        Serial1.begin(DMX_baud_rate, DMX_serial_config, DMX_revicePin, DMX_sendPin);
        digitalWrite(DMX_sendPin, LOW);
        Serial1.write(Address, 10);
        Serial1.flush(); 
        Serial1.end();
        digitalWrite(DMX_sendPin, HIGH);
    }

void pm_DMX::end()
    {
      // delete []dmxData;
      // chanSize = 0;
      // Serial1.end();
      // dmxStarted = false;
    }
enum DMX_state {IDLE, DMX_BREAK, DMX_DATA, DMX_MTBP};
static DMX_state dmx_state = DMX_BREAK;
unsigned long cur_time = 0;
void pm_DMX::update(/*bool DMX_mode*/)
    {
      if (!_dmx_init)init();
      switch(dmx_state)
      {
        case DMX_BREAK:
            //digitalWrite(GPIO_NUM_15, HIGH);
            digitalWrite(DMX_sendPin, LOW);
            delay(2);
            digitalWrite(DMX_sendPin, HIGH);
            //send data
            Serial1.begin(DMX_baud_rate, DMX_serial_config, DMX_revicePin, DMX_sendPin);
            Serial1.write(dmxData, dmxMaxChannel);
            //digitalWrite(GPIO_NUM_15, LOW); 
            dmx_state= DMX_DATA;
        break;
        case DMX_DATA:
            //digitalWrite(GPIO_NUM_15, HIGH);
            //if(millis()-cur_time>2){
            //    cur_time = millis();
                if(Serial1.availableForWrite()==512){
                    dmx_state= DMX_MTBP;
                }
            //}
            
            
        break;
        case DMX_MTBP:
            //digitalWrite(GPIO_NUM_15, HIGH);
            Serial1.flush();
            Serial1.end();
            digitalWrite(DMX_sendPin, HIGH);
            delay(2); 
            dmx_state= DMX_BREAK;
        break;
        default:
        break;
      }
      

      
      //Serial1.begin(DMX_baud_rate, DMX_serial_config, DMX_revicePin, DMX_sendPin);
      //Serial1.write(dmxData, dmxMaxChannel);
      
    }





// Function to update the DMX bus
