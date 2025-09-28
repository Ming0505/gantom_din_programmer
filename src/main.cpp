/**
 * @file main.cpp
 * @author Forairaaaaa
 * @brief
 * @version 0.1
 * @date 2023-06-06
 *
 * @copyright Copyright (c) 2023
 *
 */
#include "factory_test/factory_test.h"
#include "view/gantom_boot_splash.h"
static FactoryTest ft;
static GantomBootSplash* _bootSplash1 = nullptr;
void programmer_view_create(FactoryTest* ft);
void programmer_view_update();
void init_nvs_settings(void);
extern int _app_render_props_list_size;
void setup()
{
    pinMode(GPIO_NUM_1, INPUT_PULLUP); // motor
    ft.init();
    pinMode(GPIO_NUM_15, OUTPUT); // motor
    pinMode(GPIO_NUM_13, OUTPUT); // motor
    // Create boot splash and show it
    _bootSplash1 = new GantomBootSplash(&ft);
    _bootSplash1->show(3000); // Show for 3 seconds or until interaction
    if(digitalRead(GPIO_NUM_1)){
        _app_render_props_list_size = 5;
    }else{
        _app_render_props_list_size = 10;
    }
    programmer_view_create(&ft);
    init_nvs_settings();
}

void loop() { programmer_view_update(); }
