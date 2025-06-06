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
#include <Arduino.h>
#include <smooth_ui_toolkit.h>
#include <string.h>

using namespace SmoothUIToolKit;
using namespace SmoothUIToolKit::SelectMenu;

enum Button_state {No_active, Short_pressed, Long_pressed, Double_clicked};

struct progressBarRenderProps_t
{
    std::uint32_t theme_color;
    std::float_t progress;
    std::float_t max_progress;
    const char* tag;
};

constexpr int _dmx_channel_val_render_props_list_size = 8;
progressBarRenderProps_t _dmx_channel_val_render_props_list[] = {
    {0x88898A,   0.0f, 255.0f, "CH1"},
    {0x88898A,  50.0f, 255.0f, "CH2"},
    {0x88898A,  90.0f, 255.0f, "CH3"},
    {0x88898A, 100.0f, 255.0f, "CH4"},
    {0x88898A, 100.0f, 255.0f, "CH5"},
    {0x88898A, 100.0f, 255.0f, "CH6"},
    {0x88898A, 100.0f, 255.0f, "CH7"},
    {0x88898A, 100.0f, 255.0f, "CH8"},
};

static int _last_enc_postion = 0;
Button_state button_check(FactoryTest* ft);
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
            // Value adjustment mode
            uint32_t currentValue = _dmx_channel_val_render_props_list[_matching_index].progress;
            
            // Apply faster value changing for smoother user experience
            uint32_t change = newPos - _last_enc_postion;
            currentValue += change;
            _last_enc_postion = newPos;
            
            // Clamp to valid range (0-255)
            if (currentValue < 0) currentValue = 0;
            if (currentValue > 255) currentValue = 255;
            
            _dmx_channel_val_render_props_list[_matching_index].progress = currentValue;
        }
        
    }

    void onRender() override
    {
        // Clear
        _ft->_canvas->fillScreen(0x87C38F);

        _ft->_canvas->setTextDatum(top_center);
        _ft->_canvas->setTextColor(0x000000);
        _ft->_canvas->drawCentreString("Static Look",120, 5);
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
            _ft->_canvas->drawString(_dmx_channel_val_render_props_list[_matching_index].tag,\
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
    while(1)
    {
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
            return Long_pressed;
        }
    } else if (_isPressing) {
        // Button released
        pressDuration = millis() - _pressStartTime;
        _isPressing = false;
        
        if (pressDuration > 800) {
            // Long press - exit menu
            ft->_tone(1200, 100);
            return Long_pressed;
        }else {
            // Check for double click based on timing
            if (millis() - _lastClickTime < 300) {
                // Double-click detected
                ft->_tone(2000, 50);
                return Double_clicked;
            } else {
                //click detected 
                ft->_tone(2500, 50);
                _lastClickTime = millis();
                return Short_pressed;
            }
        }
    }
    return No_active;
}