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
void nvs_save_dmxdata(void);
extern int _start_ch ;
extern int _max_ch;
extern uint8_t dmxData[512];
using namespace SmoothUIToolKit;
using namespace SmoothUIToolKit::SelectMenu;

enum Button_state {No_active, Short_pressed, Long_pressed, Double_clicked};
static pm_DMX* _pm_dmx = nullptr;
struct progressBarRenderProps_t
{
    std::uint32_t theme_color;
    std::float_t progress;
    std::float_t max_progress;
    const char* tag;
};

constexpr int _dmx_channel_val_render_props_list_size = 8;
progressBarRenderProps_t _dmx_channel_val_render_props_list[] = {
    {0x88898A,   0.0f, 255.0f, "CH"},
    {0x88898A,  50.0f, 255.0f, "CH"},
    {0x88898A,  90.0f, 255.0f, "CH"},
    {0x88898A, 100.0f, 255.0f, "CH"},
    {0x88898A, 100.0f, 255.0f, "CH"},
    {0x88898A, 100.0f, 255.0f, "CH"},
    {0x88898A, 100.0f, 255.0f, "CH"},
    {0x88898A, 100.0f, 255.0f, "CH"},
};

extern int _start_ch;
static int _last_enc_postion = 0;
static bool show_massage = false;
static int show_massage_timout = 0;
Button_state button_check(FactoryTest* ft);
void init_button_check();
uint32_t change_speed=0;
uint32_t auto_change_time=0;
int dmx_val_auto_change = 0;
int dmx_val_change_factor=0;
bool set_channel = false;
class PM_StaticLookMenu : public SmoothOptions
{
    bool _wait_button_released = false;
    bool _is_pressing = false;
    int _matching_index = 0;
    bool _state = false; //progress bar selected state false->unselected , true->selected
    FactoryTest* _ft;
    bool _isActive = false;
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
                _state=!_state;
            break;
            case Long_pressed:
                //exit this menu
                _isActive = false;
            break;
            case Double_clicked:
                show_massage_timout = millis();
                set_channel = true;
                nvs_save_dmxdata();
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
            dmx_val_auto_change=0;
        }
        else
        {
            // Value adjustment mode
            int32_t currentValue = _dmx_channel_val_render_props_list[_matching_index].progress;
            
            // Apply faster value changing for smoother user experience
            int32_t change = newPos - _last_enc_postion;
            if(change != 0){
                int32_t delt_val = millis()-change_speed;
                change_speed = millis();
                if(delt_val<=100)
                {
                    if(dmx_val_change_factor<4)dmx_val_change_factor++;
                    else{
                        if(change>0)
                        {
                            dmx_val_auto_change = 1;//auto increase
                        }
                        else{
                            dmx_val_auto_change = 2;//auto decrease
                        }
                    }
                }else{
                    dmx_val_change_factor = 0;
                    dmx_val_auto_change = 0;
                }
            }
            if(millis()-auto_change_time>5){
                auto_change_time = millis();
                if(dmx_val_auto_change==1)change=1;
                else if(dmx_val_auto_change==2)change=-1;
            }
            currentValue += change;
            _last_enc_postion = newPos;
            
            // Clamp to valid range (0-255)
            if (currentValue < 0) currentValue = 0;
            if (currentValue > 255) currentValue = 255;
            
            _pm_dmx->write(_matching_index,currentValue);
            _dmx_channel_val_render_props_list[_matching_index].progress = currentValue;
        }
        
    }

    void onRender() override
    {
        // Clear
        _ft->_canvas->fillScreen(TFT_WHITE);
        _ft->_canvas->fillScreen(0x87C38F);
        _ft->_canvas->setTextSize(1);
        _ft->_canvas->setTextDatum(top_center);
        _ft->_canvas->setTextColor(0x000000);
        _ft->_canvas->drawCentreString("SET DEFAULT OUTPUT",120, 5);
        // Render options
        int y_offset = 6;
        for (int i = getKeyframeList().size() - 1; i >= 0; i--)
        {
            getMatchingOptionIndex(i, _matching_index);
            uint32_t theme_color = 0x000000;
            if(_state == true && i==0)theme_color = 0xff0000;
            // Render Selected Bar
            float progress_ratio = _dmx_channel_val_render_props_list[_matching_index].progress/ \
                                   _dmx_channel_val_render_props_list[_matching_index].max_progress;
            int option_width = progress_ratio * getOptionCurrentFrame(_matching_index).w;
            _ft->_canvas->drawRoundRect(getOptionCurrentFrame(_matching_index).x-2,
                                        getOptionCurrentFrame(_matching_index).y-2,
                                        getOptionCurrentFrame(_matching_index).w+4,
                                        getOptionCurrentFrame(_matching_index).h+4,
                                        5,
                                        theme_color);
            
            theme_color = _dmx_channel_val_render_props_list[_matching_index].theme_color;
            if(_state == true && i==0)theme_color = 0x00ff00;
            _ft->_canvas->fillSmoothRoundRect(getOptionCurrentFrame(_matching_index).x,
                                              getOptionCurrentFrame(_matching_index).y,
                                              option_width,
                                              getOptionCurrentFrame(_matching_index).h,
                                              5,
                                              theme_color);
            // Render Tag
            int tag_x=getOptionCurrentFrame(_matching_index).x+20;
            int tag_y=getOptionCurrentFrame(_matching_index).y;
 
            _ft->_canvas->setTextColor(0x000000);
            String tag_str = _dmx_channel_val_render_props_list[_matching_index].tag + String((int)(_matching_index+_start_ch));
            _ft->_canvas->drawString(tag_str,\
             tag_x, tag_y);

            // Render unselected bar
            if (!isOpening())
            {
                if (i <= 1)
                {
                    y_offset = getOptionCurrentFrame(_matching_index).y + 16 -
                               std::abs(getOptionCurrentFrame(_matching_index).y - getKeyframe(0).y) * 10 / 75;
                    if (isPressing())
                        y_offset = i == 0 ? getKeyframe(0).y + 16 : getKeyframe(1).y + 6;
                }
                else
                    y_offset = getOptionCurrentFrame(_matching_index).y + 6;

            }

            // Render progress value
            if (i == 0 && !isOpening())
            {
                _ft->_canvas->setTextColor(0x000000);
                String s = String((int)(_dmx_channel_val_render_props_list[_matching_index].progress));
                _ft->_canvas->drawString(s, 200, 40);
            }

            // Render notific message
            if(millis() - show_massage_timout <2000)
            {
                _ft->_canvas->fillSmoothRoundRect(0, 50, 240, 30, 0,TFT_SILVER);
                _ft->_canvas->setTextColor(TFT_BLACK);
                _ft->_canvas->setTextSize(1);
                _ft->_canvas->drawCentreString("SAVE SUCCESSED",120, 52);
            }

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

static PM_StaticLookMenu* _launcher_menu = nullptr;
unsigned long _dmxTime = 0;
uint32_t time_static_look = 0;
void pm_staticlookMenu_task(FactoryTest* ft)
{
    
    ft->_enc.setPosition(_last_enc_postion);
    ft->_canvas->setFont(&fonts::efontCN_24);
    ft->_canvas->setTextSize(1);

    // Create menu
    _launcher_menu = new PM_StaticLookMenu;
    _launcher_menu->set_ftptr(ft);
    _launcher_menu->set_active(true);

    // Selected one
    _launcher_menu->addOption();
    _launcher_menu->setLastKeyframe({25, 40, 200, 20});

    // Waiting line
    for (int i = 0; i < _dmx_channel_val_render_props_list_size - 2; i++)
    {
        // I'm too lazy to use userdata to paas the props
        _launcher_menu->addOption();
        _launcher_menu->setLastKeyframe({88 + 55 * i, 91, 40, 20});
    }

    // Set the last one next to selected one to smooth the loop
    _launcher_menu->addOption();
    _launcher_menu->setLastKeyframe({25 + 200 + 25, 40, 50, 20});

    // Config
    _launcher_menu->setConfig().renderInterval = 20;
    _launcher_menu->setConfig().readInputInterval = 50;
    _launcher_menu->setPositionDuration(600);
    _launcher_menu->setPositionTransitionPath(EasingPath::easeOutBack);
    _launcher_menu->setShapeDuration(400);

    
    for(uint8_t i=0;i<8;i++)
    {
        _dmx_channel_val_render_props_list[i].progress=(float)dmxData[_start_ch+i];
    }
    while(1)
    {
       _pm_dmx->update();
       if (millis() - time_static_look > 100)
        {
            _pm_dmx->keep_alive();
            time_static_look = millis();
        }
       if(set_channel)
        {
            set_channel = false;
            _pm_dmx->set_channel();
        }
        _launcher_menu->update(millis());
        if(!_launcher_menu->get_active()){
            delete _launcher_menu;
            break;
        }
    }
}

bool _waitingRelease = false;
bool _isPressing = false;
unsigned long _pressStartTime = 0;
 // For checking button double click
unsigned long _lastClickTime = 0;
unsigned long _doubleClickTime = 0;
bool _first_click = false;
void init_button_check()
{
    _waitingRelease = false;
    _isPressing = false;
    _pressStartTime = 0;
 // For checking button double click
    _lastClickTime = 0;
    _doubleClickTime = 0;
    _first_click = false;
}
Button_state button_check(FactoryTest* ft)
{
    // Handle button press using the factory test's button
    unsigned long pressDuration = 0;
    if (!ft->_btn_pwr.read()) { // Button is pressed when LOW
        if (!_isPressing) {
            _isPressing = true;
            _pressStartTime = millis();
        }
        pressDuration = millis() - _pressStartTime;
        if(pressDuration > 1200){
            // Long press - exit menu
            ft->_tone(1200, 100);
            _first_click = false;
            return Long_pressed;
        }
    } else if (_isPressing) {
        // Button released
        pressDuration = millis() - _pressStartTime;
        _isPressing = false;
        
        if (pressDuration > 800) {
            // Long press - exit menu
            ft->_tone(1200, 100);
            _first_click = false;
            return Long_pressed;
        }else if(pressDuration > 500){
            //click detected 
                ft->_tone(2500, 50);
                _first_click = false;
                return Short_pressed;
        }else {
            // Check for double click based on timing
            if(_first_click){
                if (millis() - _lastClickTime < 300) {
                    // Double-click detected
                    _first_click = false;
                    ft->_tone(2000, 50);
                    return Double_clicked;
                }
            }else {
                _lastClickTime = millis();
                _doubleClickTime = millis();
                _first_click=true;
            }
        }
    }else if(_first_click){
        if (millis() - _doubleClickTime > 300) {
            // Double-click detected timout
            ft->_tone(2500, 50);
            _first_click = false;
            return Short_pressed;
        }
    }
    return No_active;
}

void pm_staticlookMenu_set_pmdmxptr(pm_DMX* pm_dmx)
{
    _pm_dmx = pm_dmx;
}