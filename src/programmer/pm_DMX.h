#pragma once
#include <inttypes.h>



#define DMX_baud_rate           9600 // DMXSPEED
#define DMX_serial_config       SERIAL_8N2 //数据位8 无校验 2停止
#define DMX_break_speed         3200 //BREAKSPEED
#define DMX_break_serial_config SERIAL_8N1 //数据位8 无校验 1停止
#define DMX_sendPin             GPIO_NUM_2 //dafault on ESP8266
#define DMX_revicePin           GPIO_NUM_1

#define dmxMaxChannel 512
#define defaultMax 32

class pm_DMX 
{
    uint8_t dmxData[dmxMaxChannel] = {};
    bool _dmx_init = false;
    int chanSize =10;
    
public:
  void init();
  uint8_t read(int Channel);
  void write(int channel, uint8_t value);
  void writeAddress(int address);
  void update(/*bool DMX_mode*/);
  void end();
};

