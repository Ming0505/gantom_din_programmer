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
const String option_text[] ={"toogle Silent Boot","LOCK 9600 baud","UNLOCK 9600 baud"};
const String message_text[] ={"toogle Silent Boot successed","LOCK 9600 baud successed","UNLOCK 9600 baud successed"};
static int _last_enc_postion = 0;
static int show_massage_timout = 0;
static int show_massage_idx = 0;
Button_state button_check(FactoryTest* ft);
class PM_SettingsMenu : public SmoothSelector
{
    bool _isActive = false;
    FactoryTest* _ft = nullptr;
    void onReadInput() override
    {
        if (isOpening())
            return;
        if(millis() - show_massage_timout <2000)return;
        // Update navigation
        _ft->_check_encoder(true);
         // Get current encoder position
        int newPos = _ft->_enc.getPosition();
        switch(button_check(_ft))
        {
            case Short_pressed:
                show_massage_timout=millis();
                show_massage_idx=getSelectedOptionIdx();
                if(getSelectedOptionIdx()==0){
                    //show_massage_idx=0;
                    _pm_dmx->write_toggle_Silent_Boot();
                }
                else if(getSelectedOptionIdx()==1){
                    //show_massage_idx=1;
                    _pm_dmx->write_LOCK_9600_baud();
                }
                else if(getSelectedOptionIdx()==2){
                    //show_massage_idx=0;
                    _pm_dmx->write_UNLOCK_9600_baud();
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
        if (newPos != _last_enc_postion)
        {
            if (newPos < _last_enc_postion)
            {
                goLast();
            }
            else if (newPos > _last_enc_postion)
            {
                goNext();
            }
            _last_enc_postion = newPos;
        }

    }

    void onRender() override
    {
        // Clear
        _ft->_canvas->fillScreen(TFT_WHITE);

        _ft->_canvas->setTextDatum(top_center);
        _ft->_canvas->setTextColor(0x000000);
        _ft->_canvas->setTextSize(1);
        _ft->_canvas->drawCentreString("SILENT BOOTUP TOGGLE",120, 5);
        // Redner options
        int index = 0;
        _ft->_canvas->setColor(TFT_BLUE);

        for (auto& i : getOptionList())
        {
            _ft->_canvas->fillSmoothRoundRect(i.keyframe.x, i.keyframe.y, i.keyframe.w, i.keyframe.h, 5,TFT_CYAN);
            _ft->_canvas->setTextColor(TFT_BLACK);
            _ft->_canvas->setTextSize(0.9);
            _ft->_canvas->drawCentreString(option_text[index],i.keyframe.x+100, i.keyframe.y+2);
            index++;
        }
        // Render selector
        _ft->_canvas->setColor(TFT_RED);
        _ft->_canvas->drawRoundRect(
            getSelectorCurrentFrame().x, getSelectorCurrentFrame().y, getSelectorCurrentFrame().w, getSelectorCurrentFrame().h,5);
        // Render notific message
        if(millis() - show_massage_timout <2000)
        {
            _ft->_canvas->fillSmoothRoundRect(0, 60, 240, 30, 0,TFT_SILVER);
            _ft->_canvas->setTextColor(TFT_BLACK);
            _ft->_canvas->setTextSize(0.8);
            _ft->_canvas->drawCentreString(message_text[show_massage_idx],120, 62);
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

static PM_SettingsMenu* _launcher_menu = nullptr;

SmoothUIToolKit::Vector4D_t setting_vect_opt[]={
    {20,  40, 200, 27},
    {20,  70, 200, 27},
    {20, 100, 200, 27},
};
uint32_t time_settings = 0;
void pm_settingsMenu_task(FactoryTest* ft)
{
    ft->_enc.setPosition(_last_enc_postion);
    ft->_canvas->setFont(&fonts::efontCN_24);
    ft->_canvas->setTextSize(1);

    // Create menu
    _launcher_menu = new PM_SettingsMenu();

    _launcher_menu->set_ftptr(ft);
    _launcher_menu->set_active(true);

    _launcher_menu->setCameraSize(240,135);

    _launcher_menu->getSelectorPostion().setDuration(400);
    _launcher_menu->getSelectorShape().setTransitionPath(EasingPath::easeOutBack);

    SmoothSelector::OptionProps_t opt[3];
    
    for (int i = 0; i < 3; i++)
    {
        // I'm too lazy to use userdata to paas the props
        opt[i].keyframe = setting_vect_opt[i];
        opt[i].userData = nullptr;
        _launcher_menu->addOption(opt[i]);
    }


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
    }
}
