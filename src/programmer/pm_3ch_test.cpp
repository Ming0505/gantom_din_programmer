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
#include "../factory_test/factory_test.h"
#include "pm_DMX.h"
#include <Arduino.h>
#include <smooth_ui_toolkit.h>
#include <string.h>

using namespace SmoothUIToolKit;
using namespace SmoothUIToolKit::SelectMenu;

enum Button_state {No_active, Short_pressed, Long_pressed, Double_clicked};
static pm_DMX* _pm_dmx = nullptr;
static int _last_enc_postion = 0;
static int show_massage_timout = 0;
static int ch3_test_step = 0;
Button_state button_check(FactoryTest* ft);
class PM_ch3_test : public SmoothSelector
{
    bool _isActive = false;
    FactoryTest* _ft = nullptr;
    void onReadInput() override
    {
        if (isOpening())
            return;
        switch(button_check(_ft))
        {
            case Short_pressed:
                ch3_test_step++;
                if(ch3_test_step>4)ch3_test_step=0;
                for(uint8_t i=0;i<8;i++)
                {
                    _pm_dmx->write(i,0);
                }
                switch(ch3_test_step)
                {
                    case 0:
                        _pm_dmx->write(0,255);
                    break;
                    case 1:
                        _pm_dmx->write(1,255);
                    break;
                    case 2:
                        _pm_dmx->write(2,255);
                    break;
                    case 3:
                        _pm_dmx->write(0,255);
                        _pm_dmx->write(1,255);
                        _pm_dmx->write(2,255);
                    break;
                    case 4:
                    break;
                    default:
                    break;
                }
            break;
            case Long_pressed:
                //exit this menu
                _isActive = false;
            break;
            case Double_clicked:
            break;
            default:
            break;
        }

    }

    void onRender() override
    {
        // Clear
        _ft->_canvas->fillScreen(TFT_WHITE);

        _ft->_canvas->setTextDatum(top_center);
        _ft->_canvas->setTextColor(0x000000);
        _ft->_canvas->setTextSize(1);
        if(ch3_test_step == 0){
            _ft->_canvas->drawCentreString("STEP 1",120, 0);
            _ft->_canvas->drawCentreString("CH1: 255",120, 50);
            _ft->_canvas->drawString("CH2:   0",60, 70);
            _ft->_canvas->drawString("CH3:   0",180, 70);
        }else if(ch3_test_step == 1){
            _ft->_canvas->drawCentreString("STEP 2",120, 0);
            _ft->_canvas->drawCentreString("CH1:   0",120, 50);
            _ft->_canvas->drawString("CH2: 255",60, 70);
            _ft->_canvas->drawString("CH3:   0",180, 70);
        }else if(ch3_test_step == 2){
            _ft->_canvas->drawCentreString("STEP 3",120, 0);
            _ft->_canvas->drawCentreString("CH1:   0",120, 50);
            _ft->_canvas->drawString("CH2:   0",60, 70);
            _ft->_canvas->drawString("CH3: 255",180, 70);
        }else if(ch3_test_step == 3){
            _ft->_canvas->drawCentreString("STEP 4",120, 0);
            _ft->_canvas->drawCentreString("CH1: 255",120, 50);
            _ft->_canvas->drawString("CH2: 255",60, 70);
            _ft->_canvas->drawString("CH3: 255",180, 70);
        }else if(ch3_test_step == 4){
            _ft->_canvas->drawCentreString("STEP 5",120, 0);
            _ft->_canvas->drawCentreString("CH1:   0",120, 50);
            _ft->_canvas->drawString("CH2:   0",60, 70);
            _ft->_canvas->drawString("CH3:   0",180, 70);
        }

        // Push
        _ft->_canvas_update();
    }

    void onPress() override
    {
        // Set press anim
    }

    void onClick() override
    {
        
    }

    void onOpenEnd() override
    {

    }
public:
    void set_ftptr(FactoryTest* ft)
    {
        _ft=ft;
    }
    FactoryTest* get_ftptr()
    {
        return _ft;
    }
    void set_active(bool _active)
    {
        _isActive=_active;
    }
    bool get_active()
    {
        return _isActive;
    }

};


static PM_ch3_test* _launcher_menu = nullptr;

extern uint32_t time_settings;
void pm_3ch_test_task(FactoryTest* ft)
{
    // Create menu
    _launcher_menu = new PM_ch3_test();
    _launcher_menu->set_ftptr(ft);
    _launcher_menu->set_active(true);
    ft->_enc.setPosition(_last_enc_postion);
    ft->_canvas->setFont(&fonts::efontCN_24);
    ft->_canvas->setTextSize(1);

    ch3_test_step = 0;
    time_settings = millis();
    for(uint8_t i=0;i<8;i++)
    {
        _pm_dmx->write(i,0);
    }
    _pm_dmx->write(0,255);
    while(1)
    {
        _launcher_menu->update(millis());
        if(!_launcher_menu->get_active()){
            delete _launcher_menu;
            break;
        }
        if (millis() - time_settings > 100)
        {
         _pm_dmx->keep_alive();
         time_settings = millis();
        }
        _pm_dmx->update();
    }
}

void pm_3ch_test_set_pmdmxptr(pm_DMX* pm_dmx)
{
    _pm_dmx = pm_dmx;
}