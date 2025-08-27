#include <Arduino.h>
#include "pm_DMX.h"
extern int _start_ch ;
extern int _max_ch;
uint8_t dmxData[512] = {};
//
#define DATA_P           GPIO_NUM_13
#define DATA_N           GPIO_NUM_15

void data_high(){
    digitalWrite(GPIO_NUM_13, HIGH);
    digitalWrite(GPIO_NUM_15, LOW);
}

void data_low(){
    digitalWrite(GPIO_NUM_13, LOW);
    digitalWrite(GPIO_NUM_15, HIGH);
}
void gpio_uart_send_byte(uint8_t data){
    data_low();
    ets_delay_us(104);
    for(uint8_t i=0;i<8;i++)
    {
        if((data & (0x01<<i))>0)data_high();
        else data_low();
        ets_delay_us(104);
    }
    data_high();
    ets_delay_us(208);
}
void gpio_uart_send_bytes(uint8_t *buffer, size_t size){
    for(uint8_t i=0;i<size;i++)
    {
        gpio_uart_send_byte(buffer[i]);
    }
}
void pm_DMX::init()
    {
        if(!_dmx_init)
        {
            Serial1.setTxBufferSize(512);
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
      uint16_t _ch_val = Channel+_start_ch;
      if (!_dmx_init)init();

      if (_ch_val < 1)_ch_val = 1;
        
      if (_ch_val > 127)_ch_val = 127;
        
      if (value < 0)value = 0;
        
      if (value > 255)value = 255;
        
      dmxData[_ch_val] = value;
    }
uint8_t toggleSilent[10] = {0x91, 0x20, 0x11, 0x12, 0xAB, 0xCD, 0x34, 0xAB, 0xCD, 0}; // toggle silent mode command
uint8_t Address[10] = {0x91, 0x20, 0x11, 0xAA, 0, 1, 0x55, 0, 1, 0};    // Address setting command
uint8_t lock_9600[10] = {0x91, 0x20, 0x11, 0x12, 0x4c, 0x4f, 0x34, 0x43, 0x4b, 0};
uint8_t unlock_9600[10] = {0x91, 0x20, 0x11, 0x12, 0x75, 0x6e, 0x34, 0x6c, 0x6b, 0};
uint8_t KeepAlive[10] = {0x91, 0x20, 0x11, 1, 1, 1, 1, 1, 1, 0};
uint8_t setChannel[10] = {0x91, 0x20, 0x11, 0x77, 0, 1, 0x33, 0, 1, 0}; // Standalone setting command
void pm_DMX::writeAddress(int address)
    {
        uint8_t check_sum=0;
        if(address<1 || address>512)address=1;
        Address[4] = (address>>8) & 0Xff;
        Address[7] = (address>>8) & 0Xff;
        Address[5] = address & 0Xff;
        Address[8] = address & 0Xff;
        for(int i=0;i<9;i++){
            check_sum+=Address[i];
        }
        Address[9]=check_sum;
        //digitalWrite(DMX_sendPin, LOW);
        //delay(2);
        //digitalWrite(DMX_sendPin, HIGH);

        //Serial1.begin(DMX_baud_rate, DMX_serial_config, DMX_revicePin, DMX_sendPin);
        //digitalWrite(DMX_sendPin, LOW);
        //Serial1.write(Address, 10);
        //Serial1.flush(); 
        //Serial1.end();
        //digitalWrite(DMX_sendPin, HIGH);

        data_low();
        delay(2);
        data_high();
        ets_delay_us(525);
        gpio_uart_send_bytes(Address, 10);
    }

void pm_DMX::write_toggle_Silent_Boot(void)
    {
        uint8_t check_sum=0;
        for(int i=0;i<9;i++){
            check_sum+=toggleSilent[i];
        }
        toggleSilent[9]=check_sum;
        //digitalWrite(DMX_sendPin, LOW);
        //delay(2);
        //digitalWrite(DMX_sendPin, HIGH);

        //Serial1.begin(DMX_baud_rate, DMX_serial_config, DMX_revicePin, DMX_sendPin);
        //digitalWrite(DMX_sendPin, LOW);
        //Serial1.write(toggleSilent, 10);
        //Serial1.flush(); 
        //Serial1.end();
        //digitalWrite(DMX_sendPin, HIGH);

        data_low();
        delay(2);
        data_high();
        ets_delay_us(525);
        gpio_uart_send_bytes(toggleSilent, 10);
    }

void pm_DMX::write_LOCK_9600_baud(void)
    {
        uint8_t check_sum=0;
        for(int i=0;i<9;i++){
            check_sum+=lock_9600[i];
        }
        lock_9600[9]=check_sum;
        //digitalWrite(DMX_sendPin, LOW);
        //delay(2);
        //digitalWrite(DMX_sendPin, HIGH);
//
        //Serial1.begin(DMX_baud_rate, DMX_serial_config, DMX_revicePin, DMX_sendPin);
        //digitalWrite(DMX_sendPin, LOW);
        //Serial1.write(lock_9600, 10);
        //Serial1.flush(); 
        //Serial1.end();
        //digitalWrite(DMX_sendPin, HIGH);

        data_low();
        delay(2);
        data_high();
        ets_delay_us(525);
        gpio_uart_send_bytes(lock_9600, 10);
    }

void pm_DMX::write_UNLOCK_9600_baud(void)
    {
        uint8_t check_sum=0;
        for(int i=0;i<9;i++){
            check_sum+=unlock_9600[i];
        }
        unlock_9600[9]=check_sum;
        //digitalWrite(DMX_sendPin, LOW);
        //delay(2);
        //digitalWrite(DMX_sendPin, HIGH);

        //Serial1.begin(DMX_baud_rate, DMX_serial_config, DMX_revicePin, DMX_sendPin);
        //digitalWrite(DMX_sendPin, LOW);
        //Serial1.write(unlock_9600, 10);
        //Serial1.flush(); 
        //Serial1.end();
        //digitalWrite(DMX_sendPin, HIGH);

        data_low();
        delay(2);
        data_high();
        ets_delay_us(525);
        gpio_uart_send_bytes(unlock_9600, 10);
    }

void pm_DMX::keep_alive(void)
    {
        uint8_t check_sum=0;
        for(int i=0;i<9;i++){
            check_sum+=KeepAlive[i];
        }
        KeepAlive[9]=check_sum;

        //digitalWrite(DMX_sendPin, LOW);
        //delay(2);
        //digitalWrite(DMX_sendPin, HIGH);

        //Serial1.begin(DMX_baud_rate, DMX_serial_config, DMX_revicePin, DMX_sendPin);
        ////digitalWrite(DMX_sendPin, LOW);
        //Serial1.write(KeepAlive, 10);
        //while(!(Serial1.availableForWrite()==640));
        //Serial1.flush(); 
        //Serial1.end();
        //digitalWrite(DMX_sendPin, HIGH);
        data_low();
        delay(2);
        data_high();
        ets_delay_us(525);
        gpio_uart_send_bytes(KeepAlive, 10);
    }

void pm_DMX::set_channel(void)
    {
        uint8_t check_sum=0;
        for(int i=0;i<9;i++){
            check_sum+=setChannel[i];
        }
        setChannel[9]=check_sum;
        data_low();
        delay(2);
        data_high();
        ets_delay_us(525);
        gpio_uart_send_bytes(setChannel, 10);
    }

void pm_DMX::end()
    {
      // delete []dmxData;
      // chanSize = 0;
      // Serial1.end();
      // dmxStarted = false;
    }
enum DMX_state {IDLE, DMX_BREAK, DMX_BREAK_WAIT,DMX_DATA, DMX_MTBP,DMX_MTBP_WAIT,KEEP_ALIVE};
static DMX_state dmx_state = DMX_BREAK;
unsigned long cur_time = 0;
void pm_DMX::update(/*bool DMX_mode*/)
    {
      if (!_dmx_init)init();
      switch(dmx_state)
      {
        case DMX_BREAK:
            //digitalWrite(GPIO_NUM_15, HIGH);
            //digitalWrite(DMX_sendPin, LOW);
            data_low();
            cur_time = millis();
            dmx_state= DMX_BREAK_WAIT;
        break;
        case DMX_BREAK_WAIT:
            if(millis()-cur_time>2){
                //digitalWrite(DMX_sendPin, HIGH);
                data_high();
                ets_delay_us(525);
                //send data
                //Serial1.begin(DMX_baud_rate, DMX_serial_config, DMX_revicePin, DMX_sendPin);
                //Serial1.write(dmxData, 10);
                gpio_uart_send_bytes(dmxData, 10);
                dmx_state= DMX_DATA;
            }
        break;
        case DMX_DATA:
                data_high();
                //if(Serial1.availableForWrite()==640){
                    dmx_state= DMX_MTBP;
                //}
        break;
        case DMX_MTBP:
            //Serial1.flush();
            //Serial1.end();
            //digitalWrite(DMX_sendPin, HIGH);
            cur_time = millis();
            dmx_state= DMX_MTBP_WAIT;
        break;
        case DMX_MTBP_WAIT:
            if(millis()-cur_time>2){
                //digitalWrite(DMX_sendPin, LOW);
                data_low();
                dmx_state= DMX_BREAK;
            }
        break;
        default:
        break;
      }
      

      
      //Serial1.begin(DMX_baud_rate, DMX_serial_config, DMX_revicePin, DMX_sendPin);
      //Serial1.write(dmxData, dmxMaxChannel);
      
    }





// Function to update the DMX bus
