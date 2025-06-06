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

static FactoryTest ft;

void programmer_view_create(FactoryTest* ft);
void programmer_view_update();

void setup()
{
    ft.init();
    programmer_view_create(&ft);
}

void loop() { programmer_view_update(); }
