/**
 * @file view.cpp
 * @author Forairaaaaa
 * @brief
 * @version 0.1
 * @date 2024-01-08
 *
 * @copyright Copyright (c) 2024
 *
 */
#include "../factory_test/factory_test.h"
#include "../programmer/pm_dmx_address.h"
#include "../programmer/pm_DMX.h"
#include "assets/assets.h"
#include <Arduino.h>
#include <smooth_ui_toolkit.h>


static FactoryTest* _ft = nullptr;
static pm_DMX* _pm_dmx = nullptr;

using namespace SmoothUIToolKit;
using namespace SmoothUIToolKit::SelectMenu;


void init_button_check();
struct AppOptionRenderProps_t
{
    std::uint32_t theme_color;
    std::uint32_t tag_color;
    const char* tag;
    const char* tag_string;
};
constexpr int _fa2_app_render_props_list_size = 7;

constexpr AppOptionRenderProps_t _fa2_app_render_props_list[] = {
    {0xCEDB20, 0x4E5B38, "1CH SET", "1CH.."},
    {0xCE20B8, 0x4E5B38, "2CH SET", "2CH.."},
    {0x20DBB8, 0x4E5B38, "3CH SET", "3CH.."},
    {0x20DB20, 0x4E5B38, "4CH SET", "4CH.."},
    {0xA05038, 0x4E5B38, "7CH SET", "7CH.."},
    {0xB08018, 0x4E5B38, "DATA RESET", "DAT.."},
    {0xB00000, 0x4E5B38, "EXIT", "EXI"},
};

const String notify_str_list[]={
    "SETUP 1CH",
    "SETUP 2CH",
    "SETUP 3CH",
    "SETUP 4CH",
    "SETUP 7CH",
    "Setting Clear"
};

uint32_t _dmx_keeplive_time_count = 0;
static int _last_enc_postion = 0;
static bool _is_just_boot_in = true;
uint32_t set_channel_time=0;
bool set_channel_flag=0;
uint32_t notify_str=0;
bool notify_str_flag=false;
class LauncherMenuFA2 : public SmoothOptions
{
    bool _wait_button_released = false;
    bool _is_pressing = false;
    bool _app_close_flag = false;
    int _matching_index = 0;
    bool _isActive = true;
    void onReadInput() override
    {
        if (isOpening())
            return;

        // Update navigation
        _ft->_check_encoder(true);
        if (_ft->_enc.getPosition() != _last_enc_postion)
        {
            if (_ft->_enc.getPosition() < _last_enc_postion)
            {
                goLast();
            }
            else if (_ft->_enc.getPosition() > _last_enc_postion)
            {
                goNext();
            }

            _last_enc_postion = _ft->_enc.getPosition();
        }

        // If just boot in, lock until button released
        if (_is_just_boot_in)
        {
            // If not pressing
            if (_ft->_btn_pwr.read())
            {
                _is_just_boot_in = false;
            }
        }

        // If select
        else if (!_ft->_btn_pwr.read())
        {
            if (!_wait_button_released && !_app_close_flag)
            {
                _ft->_tone(2500, 50);

                _wait_button_released = true;
                _is_pressing = true;

                // Squeeze it
                press({0, 12, 240, 52});
            }
        }

        // Unlock if no button is pressing
        else
        {
            if(_app_close_flag){
                _app_close_flag = false;
                return;
            }
            _wait_button_released = false;
            if (_is_pressing)
            {
                _is_pressing = false;
                release();
            }
        }
    }

    void onRender() override
    {
        // Clear
        _ft->_canvas->fillScreen(TFT_WHITE);

        // Render options
        int y_offset = 6;
        _ft->_canvas->setTextDatum(top_right);
        for (int i = getKeyframeList().size() - 1; i >= 0; i--)
        {
            getMatchingOptionIndex(i, _matching_index);

            // Render cards
            _ft->_canvas->fillSmoothRoundRect(getOptionCurrentFrame(_matching_index).x,
                                              getOptionCurrentFrame(_matching_index).y,
                                              getOptionCurrentFrame(_matching_index).w,
                                              getOptionCurrentFrame(_matching_index).h,
                                              20,
                                              _fa2_app_render_props_list[_matching_index].theme_color);

            // Render icons
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
                if(i!=0)
                {
                    _ft->_canvas->setTextSize(0.7);
                    _ft->_canvas->drawString(_fa2_app_render_props_list[_matching_index].tag_string,
                                        getOptionCurrentFrame(_matching_index).x + 45,
                                        y_offset+7);
                    _ft->_canvas->setTextSize(1);
                }
                
            }

            // Render tags
            if (i == 0 && !isOpening())
            {
                _ft->_canvas->setTextColor(_fa2_app_render_props_list[_matching_index].tag_color);
                _ft->_canvas->drawString(_fa2_app_render_props_list[_matching_index].tag, 227, 26);
            }
        }
        if(notify_str_flag){
            _ft->_canvas->fillSmoothRoundRect(0, 50, 240, 30, 0,TFT_SILVER);
            //_ft->_canvas->setTextColor(TFT_BLACK);
            _ft->_canvas->setTextSize(1);
            _ft->_canvas->drawCentreString(notify_str_list[notify_str],120, 52);
        }

        // Push
        _ft->_canvas_update();
    }

    void onPress() override
    {
        // Set press anim
        setDuration(200);
        setTransitionPath(EasingPath::easeOutQuad);
    }

    void onClick() override
    {
        // Set open anim
        setDuration(300);
        setTransitionPath(EasingPath::easeOutQuad);

        open({-20, -20, 280, 175});
    }

    void onOpenEnd() override
    {
        _open_app();

        // Reset anim
        setPositionDuration(600);
        setPositionTransitionPath(EasingPath::easeOutBack);
        setShapeDuration(400);

        // Close option
        printf("close app\n");
        close();
        _ft->_enc.setPosition(_last_enc_postion);
        _ft->_canvas->setFont(&fonts::efontCN_24);
        _ft->_canvas->setTextSize(1);
    }

    void _open_app()
    {
        int matching_index = getSelectedOptionIndex();
        printf("open app\n");
        for(uint8_t i=0;i<8;i++)
        {
            _pm_dmx->write(i,0);
        }
        set_channel_time = millis();
        
        if (matching_index == 0)
        {
            _pm_dmx->write(0,255);
            notify_str = 0;
            notify_str_flag = true;
            set_channel_flag=true;
        }
        else if (matching_index == 1)
        {
            //2CH SET
            _pm_dmx->write(0,255);
            _pm_dmx->write(1,255);
            notify_str = 1;
            notify_str_flag = true;
            set_channel_flag=true;

        }
        else if (matching_index == 2)
        {
            //3CH SET
            _pm_dmx->write(0,255);
            _pm_dmx->write(1,255);
            _pm_dmx->write(2,255);
            notify_str = 2;
            notify_str_flag = true;
            set_channel_flag=true;
 
        }
        else if (matching_index == 3)
        {
            //4CH SET
            _pm_dmx->write(0,255);
            _pm_dmx->write(1,255);
            _pm_dmx->write(2,255);
            _pm_dmx->write(3,255);
            notify_str = 3;
            notify_str_flag = true;
            set_channel_flag=true;
 
        }
        else if (matching_index == 4)
        {
            //7CH SET
            _pm_dmx->write(0,255);
            _pm_dmx->write(1,255);
            _pm_dmx->write(2,255);
            _pm_dmx->write(3,255);
            _pm_dmx->write(4,255);
            _pm_dmx->write(5,255);
            _pm_dmx->write(6,255);
            _pm_dmx->write(7,255);
            notify_str = 4;
            notify_str_flag = true;
            set_channel_flag=true;

        }
        else if (matching_index == 5)
        {
            //DATA RESET
            notify_str = 5;
            notify_str_flag = true;
            set_channel_flag=true;
        }
        else if (matching_index == 6)
        {
            //EXIT TEST
            _isActive = false;
        }
            
        //_wait_button_released = true;
        //_is_pressing = true;
        init_button_check();
        _app_close_flag = true;
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

static LauncherMenuFA2* _launcher_menu = nullptr;

void fa2_view_create(FactoryTest* ft)
{
    _ft = ft;
    _ft->_enc.setPosition(_last_enc_postion);
    _ft->_canvas->setFont(&fonts::efontCN_24);
    _ft->_canvas->setTextSize(1);

    // Create menu
    _launcher_menu = new LauncherMenuFA2;

    // Selected one
    _launcher_menu->addOption();
    _launcher_menu->setLastKeyframe({6, 6, 228, 64});

    // Waiting line
    for (int i = 0; i < _fa2_app_render_props_list_size - 2; i++)
    {
        // I'm too lazy to use userdata to paas the props
        _launcher_menu->addOption();
        _launcher_menu->setLastKeyframe({88 + 65 * i, 81, 58, 44});
    }

    // Set the last one next to selected one to smooth the loop
    _launcher_menu->addOption();
    _launcher_menu->setLastKeyframe({6 + 228 + 24, 6, 58, 44});

    // Config
    _launcher_menu->setConfig().renderInterval = 20;
    _launcher_menu->setConfig().readInputInterval = 50;
    _launcher_menu->setPositionDuration(600);
    _launcher_menu->setPositionTransitionPath(EasingPath::easeOutBack);
    _launcher_menu->setShapeDuration(400);

}

void fa2_view_update()
{
    _launcher_menu->update(millis());
    _pm_dmx->update();
    // Read bat voltage
    if (millis() - _dmx_keeplive_time_count > 100)
    {
        _pm_dmx->keep_alive();
        _dmx_keeplive_time_count = millis();
    }
    if(set_channel_flag)
    {
        _pm_dmx->set_channel();
        set_channel_flag = false;
    }
    if(millis() - set_channel_time>2000)
    {
        notify_str_flag=false;
    }
}
void fa2_set_pmdmxptr(pm_DMX* pm_dmx)
{
    _pm_dmx = pm_dmx;
}
void fa2_Menu_task(FactoryTest* ft)
{
    fa2_view_create(ft);
    while(1){
        fa2_view_update();
        if(!_launcher_menu->get_active()){
            delete _launcher_menu;
            break;
        }
    }
}

