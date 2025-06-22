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
void nvs_save_dmxaddress(void);

static pm_DMX* _pm_dmx = nullptr;

constexpr int _dmx_channel_val_render_props_list_size = 5;
static int _last_enc_postion = 0;

Button_state button_check(FactoryTest* ft);
int DMX_Address_val = 0;
static int show_massage_timout = 0;
static int show_massage1_timout = 0;
class PM_DmxaddressMenu : public SmoothSelector
{
    bool _wait_button_released = false;
    bool _is_pressing = false;
    int _matching_index = 0;
    bool _state = false; //progress bar selected state false->unselected , true->selected
    bool _isActive = false;
    FactoryTest* _ft = nullptr;
    void onReadInput() override
    {
        if (isOpening())
            return;

        // Update navigation
        _ft->_check_encoder(true);
         // Get current encoder position
        int newPos = _ft->_enc.getPosition();
        switch(button_check(_ft))
        {
            case Short_pressed:
                if(getSelectedOptionIdx()!=4){
                    _state=!_state;
                }else
                {
                    //send dmx address set cmd
                    show_massage_timout=millis();
                    _pm_dmx->writeAddress(DMX_Address_val);
                }
            break;
            case Long_pressed:
                //exit this menu
                _isActive = false;
            break;
            case Double_clicked:
                show_massage1_timout=millis();
                nvs_save_dmxaddress();
            break;
            default:
            break;
        }
        if(_state == 0)
        {
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
        else
        {
            int temp_val=0;
            int change = newPos - _last_enc_postion;
            int val_100 = DMX_Address_val/100;
            int val_10 = (DMX_Address_val%100)/10;
            int val_1 = DMX_Address_val%10;
            _last_enc_postion = newPos;
            switch(getSelectedOptionIdx())
            {
                case 0:
                    temp_val = val_100+change;
                    if(temp_val>5)
                    {
                        val_100 = 5;
                    }else if(temp_val<0)
                    {
                        val_100 = 0;
                    }else{
                        val_100 = temp_val;
                    }
                    if(((val_10*10 + val_1)>11)&&(val_100>4))val_100=4;
                    DMX_Address_val = val_100*100 + val_10*10 + val_1;
                break;
                case 1:
                    temp_val = val_10+change;
                    if(temp_val>9)
                    {
                        val_10 = 9;
                    }else if(temp_val<0)
                    {
                        val_10 = 0;
                    }else{
                        val_10 = temp_val;
                    }
                    if(val_100==5 && val_10>1)val_10=1;
                    DMX_Address_val = val_100*100 + val_10*10 + val_1;
                break;
                case 2:
                    temp_val = val_1+change;
                    if(temp_val>9)
                    {
                        val_1 = 9;
                    }else if(temp_val<0)
                    {
                        val_1 = 0;
                    }else{
                        val_1 = temp_val;
                    }
                    DMX_Address_val = val_100*100 + val_10*10 + val_1;
                    if(DMX_Address_val>511)DMX_Address_val=511;
                break;
                case 3:
                    temp_val = DMX_Address_val + change;
                    if(temp_val>511)temp_val=511;
                    else if(temp_val<0)temp_val=0;
                    DMX_Address_val = temp_val;
                break;
            }

        }
    }

    void onRender() override
    {
        // Clear
        _ft->_canvas->fillScreen(TFT_WHITE);
        _ft->_canvas->setTextSize(1);
        _ft->_canvas->setTextDatum(top_center);
        _ft->_canvas->setTextColor(0x000000);
        _ft->_canvas->drawCentreString("DMX Address",120, 5);
        // Redner options
        int index = 0;
        _ft->_canvas->setColor(TFT_BLUE);

        for (auto& i : getOptionList())
        {
            if(index==4)
            {
                _ft->_canvas->fillSmoothRoundRect(i.keyframe.x, i.keyframe.y, i.keyframe.w, i.keyframe.h, 5,TFT_CYAN);
                _ft->_canvas->setTextColor(TFT_BLACK);
                _ft->_canvas->drawCentreString("Apply",i.keyframe.x+35, i.keyframe.y);
            }
            else
            {
                if(_state==1 && (getSelectedOptionIdx()==index))
                {
                    _ft->_canvas->fillSmoothRoundRect(i.keyframe.x, i.keyframe.y, i.keyframe.w, i.keyframe.h, 5,TFT_SKYBLUE);
                }

            }
            index++;
        }
        // Render options value
        index=0;
        for (auto& i : getOptionList())
        {
            String val_str;
            switch(index)
            {
                case 0:
                    val_str = String((int)(DMX_Address_val /100));
                break;
                case 1:
                    val_str = String((int)((DMX_Address_val %100)/10));
                break;
                case 2:
                    val_str = String((int)(DMX_Address_val %10));
                break;
            }
            if(index<3)
            {
                _ft->_canvas->setTextColor(TFT_BLACK);
                _ft->_canvas->drawCentreString(val_str,i.keyframe.x+15, i.keyframe.y+5);
            }
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
            _ft->_canvas->drawCentreString("apply successed",120, 62);
        }else if(millis() - show_massage1_timout <2000)
        {
            _ft->_canvas->fillSmoothRoundRect(0, 60, 240, 30, 0,TFT_SILVER);
            _ft->_canvas->setTextColor(TFT_BLACK);
            _ft->_canvas->setTextSize(0.8);
            _ft->_canvas->drawCentreString("save successed",120, 62);
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

static PM_DmxaddressMenu* _launcher_menu = nullptr;

SmoothUIToolKit::Vector4D_t vect_opt[]={
    {60,        45, 30, 40},
    {60 + 45*1, 45, 30, 40},
    {60 + 45*2, 45, 30, 40},
    {60       , 45,120, 40},
    {85       , 95, 70, 30},
};

void pm_dmxaddressMenu_task(FactoryTest* ft)
{
    ft->_enc.setPosition(_last_enc_postion);
    ft->_canvas->setFont(&fonts::efontCN_24);
    ft->_canvas->setTextSize(1);

    // Create menu
    _launcher_menu = new PM_DmxaddressMenu();

    _launcher_menu->set_ftptr(ft);
    _launcher_menu->set_active(true);

    _launcher_menu->setCameraSize(240,135);

    _launcher_menu->getSelectorPostion().setDuration(400);
    _launcher_menu->getSelectorShape().setTransitionPath(EasingPath::easeOutBack);

    SmoothSelector::OptionProps_t opt[5];
    
    for (int i = 0; i < 5; i++)
    {
        // I'm too lazy to use userdata to paas the props
        opt[i].keyframe = vect_opt[i];
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
    }
}

void pm_dmxaddressMenu_set_pmdmxptr(pm_DMX* pm_dmx)
{
    _pm_dmx = pm_dmx;
}
