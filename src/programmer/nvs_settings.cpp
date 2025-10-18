/**
 * @file pm_dmx_address.cpp
 * @author Ming
 * @brief
 * @version 0.1
 * @date 2025-06-04
 *
 * @copyright Copyright (c) 2025
 *
 */
#include <Preferences.h>

Preferences preferences;
bool saveFlag = false;
bool updateFlag = false;
unsigned int numOfReboots = 0;

uint8_t EEPROM_CN[1][8]; // used to be 64
extern int _start_ch ;
extern int _max_ch;
extern int DMX_Address_val;
extern uint8_t dmxData[512];
void init_nvs_settings(void)
{
  preferences.begin("nvs_settings", false);
  Serial.println("initFlag = " + String(preferences.getUChar("initFlag")));

  if (preferences.getUChar("initFlag") != 0xA5)
  { // uninitialized EEPROM
    Serial.println("Initializing EEPROM for first time");
    preferences.putUInt("DMX_START_CH", 1); //DMX èµ·å§‹é€ÿéÿ“å€¼
    preferences.putUInt("DMX_MAX_CH", 127);   //DMX æ”¯æÿÿçÿ„æÿ€å¤§é€ÿéÿ“æ•°
    preferences.putUInt("DMX_ADDRESS", 1);  //DMX åÿ°åÿ€å€¼
    preferences.putUInt("numOfReboots", 0);

    for (int i = 0; i < 512; i++)
    { // wipe checkpoints
      dmxData[i] = 0;
    }

    Serial.println("RESET DMX");

    preferences.putBytes("dmxData", dmxData, 512);

    preferences.putUChar("initFlag", 0xA5);
  }else if (preferences.getUChar("initFlag") == 0xA5)
  {

    preferences.putUInt("numOfReboots", preferences.getUInt("numOfReboots", 0) + 1);
    Serial.println("Loading preferences");
    _start_ch = preferences.getUInt("DMX_START_CH", 1);
    _max_ch = preferences.getUInt("DMX_MAX_CH", 127);
    DMX_Address_val = preferences.getUInt("DMX_ADDRESS", 1);  //DMX åÿ°åÿ€å€¼
    numOfReboots = preferences.getUInt("numOfReboots", 0);

    
    preferences.getBytes("dmxData", dmxData, 512);

    for (int i = 0; i < 512; i++)
    { // display dmx values
      Serial.print(" ch" + String(i) + ": " + String(dmxData[i]));
    }

    Serial.println("start address: " + String(_start_ch));
    Serial.println("reboots: " + String(numOfReboots));
  }
}

void nvs_save_dmxdata(void)
{
  preferences.putBytes("dmxData", dmxData, 512);
}

void nvs_save_dmxsettings(void)
{
  preferences.putUInt("DMX_START_CH", _start_ch); //DMX èµ·å§‹é€ÿéÿ“å€¼
  preferences.putUInt("DMX_MAX_CH", _max_ch);   //DMX æ”¯æÿÿçÿ„æÿ€å¤§é€ÿéÿ“æ•°
}

void nvs_save_dmxaddress(void)
{
  preferences.putUInt("DMX_ADDRESS", DMX_Address_val);  //DMX åÿ°åÿ€å€¼
}

void nvs_clear_settings(void)
{
  preferences.putUChar("initFlag", 0xFF);
}
