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
static pm_DMX* pm_dmx = nullptr;

using namespace SmoothUIToolKit;
using namespace SmoothUIToolKit::SelectMenu;

void pm_staticlookMenu_set_pmdmxptr(pm_DMX* pm_dmx);
void pm_staticlookMenu_task(FactoryTest* ft);
void pm_silent_boot_toggle_set_pmdmxptr(pm_DMX* pm_dmx);
void pm_silent_boot_toggle_task(FactoryTest* ft);
void pm_dmxaddressMenu_set_pmdmxptr(pm_DMX* pm_dmx);
void pm_dmxaddressMenu_task(FactoryTest* ft);
void pm_settingsMenu_task(FactoryTest* ft);
void pm_dmx_settingsMenu_task(FactoryTest* ft);
void pm_1ch_test_task(FactoryTest* ft);
void pm_1ch_test_set_pmdmxptr(pm_DMX* pm_dmx);
void pm_2ch_test_task(FactoryTest* ft);
void pm_2ch_test_set_pmdmxptr(pm_DMX* pm_dmx);
void pm_3ch_test_task(FactoryTest* ft);
void pm_3ch_test_set_pmdmxptr(pm_DMX* pm_dmx);
void pm_4ch_test_task(FactoryTest* ft);
void pm_4ch_test_set_pmdmxptr(pm_DMX* pm_dmx);
void pm_7ch_test_task(FactoryTest* ft);
void pm_7ch_test_set_pmdmxptr(pm_DMX* pm_dmx);

void init_button_check();
struct AppOptionRenderProps_t
{
    std::uint32_t theme_color;
    std::uint32_t tag_color;
    const char* tag;
    const char* tag_string;
};
constexpr int _app_render_props_list_size = 4;
constexpr int _app_render_props_list_size = 5;
constexpr AppOptionRenderProps_t _app_render_props_list[] = {
    {0xB8DBD9, 0x385B59, "SET DMX ADDRESS", "SET.."},
    {0x87C38F, 0x07430F, "SET DMX OUTPUT", "SET.."},
    {0xCEDBB8, 0x4E5B38, "SILENT BOOT TOGGLE", "SIL.."},
    {0xEB7A24, 0x4E5B10, "POWER OFF", "POW.."},
    {0xFFFF00, 0x49496E, "ADDRESS RESET", "ADD.."},
    //{0xCEDBB8, 0x4E5B38, "RESET PROGRAMMER", "RES.."},
    {0xCEDB20, 0x4E5B38, "1CH TEST", "1CH.."},
    {0xCE20B8, 0x4E5B38, "2CH TEST", "2CH.."},
    {0x20DBB8, 0x4E5B38, "3CH TEST", "3CH.."},
    {0x20DB20, 0x4E5B38, "4CH TEST", "4CH.."},
    {0x4020B8, 0x4E5B38, "7CH TEST", "7CH.."},
};
constexpr AppOptionRenderProps_t _test_app_render_props_list[] = {
    {0xEB7A24, 0x4E5B10, "POWER OFF", "POW.."},
    {0xFFFF00, 0x49496E, "FACTORY MODEL 1", "FA1.."},
    {0xB8DBD9, 0x385B59, "FACTORY MODEL 2", "FA2.."},
};

static Transition2D* _batv_panel_transition = nullptr;
static std::uint32_t _batv_time_count = 0;
static char _batv[10] = {0};
static int _last_enc_postion = 0;
static bool _is_just_boot_in = true;
uint32_t reset_success_count=0;
class LauncherMenu : public SmoothOptions
{
    bool _wait_button_released = false;
    bool _is_pressing = false;
    bool _app_close_flag = false;
    int _matching_index = 0;

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

        // Render batv panel
        //_ft->_canvas->pushImage(
        //    _batv_panel_transition->getValue().x, _batv_panel_transition->getValue().y, 83, 54, image_data_bat_panel);
        //_ft->_canvas->setTextDatum(top_left);
        //_ft->_canvas->setTextColor(0x7F5845);
        //_ft->_canvas->setFont(&fonts::efontCN_16);
        //_ft->_canvas->drawString("Bat:", _batv_panel_transition->getValue().x + 6, _batv_panel_transition->getValue().y + 13);
        //_ft->_canvas->setFont(&fonts::efontCN_24);
        //_ft->_canvas->drawString(_batv, _batv_panel_transition->getValue().x + 4, _batv_panel_transition->getValue().y + 29);

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
                                              _app_render_props_list[_matching_index].theme_color);

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
                    _ft->_canvas->drawString(_app_render_props_list[_matching_index].tag_string,
                                        getOptionCurrentFrame(_matching_index).x + 45,
                                        y_offset+7);
                    _ft->_canvas->setTextSize(1);
                }
                
                //_ft->_canvas->pushImage(getOptionCurrentFrame(_matching_index).x + 13,
                //                        y_offset,
                //                        32,
                //                        32,
                //                        _app_render_props_list[_matching_index].icon);
                // spdlog::info("{} {}", std::abs(getOptionCurrentFrame(_matching_index).y - getKeyframe(0).y), y_offset);
            }

            // Render tags
            if (i == 0 && !isOpening())
            {
                _ft->_canvas->setTextColor(_app_render_props_list[_matching_index].tag_color);
                _ft->_canvas->drawString(_app_render_props_list[_matching_index].tag, 227, 26);
            }
        }
        if(millis() - reset_success_count<800)
        {
            _ft->_canvas->fillSmoothRoundRect(0, 60, 240, 30, 0,TFT_SILVER);
            _ft->_canvas->setTextColor(TFT_BLACK);
            _ft->_canvas->drawCentreString("RESET SUCCESSED", 120, 63);
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
        if (matching_index == 0){
            printf("open app 0\n");
            pm_dmxaddressMenu_task(_ft);
        }  
        else if (matching_index == 1){
            printf("open app 1\n");
            pm_staticlookMenu_task(_ft);
        }
        else if (matching_index == 2)
        {
            pm_silent_boot_toggle_task(_ft);
        } 
        else if (matching_index == 3)
        {
            //pm_settingsMenu_task(_ft);
            _ft->_power_off();
            esp_restart();
        }  
        else if (matching_index == 4)
        {
            //pm_dmx_settingsMenu_task(_ft);
            pm_dmx->writeAddress(1);
            reset_success_count = millis();

        }
        else if (matching_index == 5)
        {
            //1CH TEST
            pm_1ch_test_task(_ft);
        }
        else if (matching_index == 6)
        {
            //2CH TEST
            pm_2ch_test_task(_ft);
        }
        else if (matching_index == 7)
        {
            //3CH TEST
            pm_3ch_test_task(_ft);
        }
        else if (matching_index == 8)
        {
            //4CH TEST
            pm_4ch_test_task(_ft);
        }
        else if (matching_index == 9)
        {
            //7CH TEST
            pm_7ch_test_task(_ft);
        }
            
        //_wait_button_released = true;
        //_is_pressing = true;
        init_button_check();
        _app_close_flag = true;
    }
};

static LauncherMenu* _launcher_menu = nullptr;

void programmer_view_create(FactoryTest* ft)
{
    _ft = ft;
    _ft->_enc.setPosition(_last_enc_postion);
    _ft->_canvas->setFont(&fonts::efontCN_24);
    _ft->_canvas->setTextSize(1);

    // Create menu
    _launcher_menu = new LauncherMenu;

    // Selected one
    _launcher_menu->addOption();
    _launcher_menu->setLastKeyframe({6, 6, 228, 64});

    // Waiting line
    for (int i = 0; i < _app_render_props_list_size - 2; i++)
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

    pm_dmx = new pm_DMX();
    pm_dmx->init();
    pm_staticlookMenu_set_pmdmxptr(pm_dmx);
    pm_dmxaddressMenu_set_pmdmxptr(pm_dmx);
    pm_silent_boot_toggle_set_pmdmxptr(pm_dmx);
    pm_1ch_test_set_pmdmxptr(pm_dmx);
    pm_2ch_test_set_pmdmxptr(pm_dmx);
    pm_3ch_test_set_pmdmxptr(pm_dmx);
    pm_4ch_test_set_pmdmxptr(pm_dmx);
    pm_7ch_test_set_pmdmxptr(pm_dmx);
}

void programmer_view_update()
{
    _launcher_menu->update(millis());
    //_batv_panel_transition->update(millis());

    // Read bat voltage
    if (millis() - _batv_time_count > 100)
    {
        pm_dmx->keep_alive();
        _batv_time_count = millis();
    }
}

