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

static pm_DMX* _pm_dmx = nullptr;
class PM_Silent_boot_toggle : public SmoothSelector
{
    bool _isActive = false;
    FactoryTest* _ft = nullptr;
    void onReadInput() override
    {
        if (isOpening())
            return;

    }

    void onRender() override
    {
        // Clear
        _ft->_canvas->fillScreen(TFT_WHITE);

        _ft->_canvas->setTextDatum(top_center);
        _ft->_canvas->setTextColor(0x000000);
        _ft->_canvas->setTextSize(1);
        _ft->_canvas->drawCentreString("Power cycling ",120, 15);
        _ft->_canvas->drawCentreString("output Observe if",120, 40);
        _ft->_canvas->drawCentreString("address is silent",120, 65);
        _ft->_canvas->drawCentreString("upon bootup",120, 90);

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


static PM_Silent_boot_toggle* _launcher_menu = nullptr;

extern uint32_t time_settings;
void pm_silent_boot_toggle_task(FactoryTest* ft)
{
    // Create menu
    _launcher_menu = new PM_Silent_boot_toggle();
    _launcher_menu->set_ftptr(ft);
    _launcher_menu->set_active(true);
    ft->_canvas->setFont(&fonts::efontCN_24);
    ft->_canvas->setTextSize(1);

    uint16_t exit_counter = 0;

    _pm_dmx->write_toggle_Silent_Boot();
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
         exit_counter++;
         if(exit_counter>30){
            exit_counter=0;
            _launcher_menu->set_active(false);
         }
        }
    }
}

void pm_silent_boot_toggle_set_pmdmxptr(pm_DMX* pm_dmx)
{
    _pm_dmx = pm_dmx;
}