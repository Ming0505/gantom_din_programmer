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

#define MOTOR_PIN 15
#define MOTOR_ON HIGH
#define MOTOR_OFF LOW
#define FIXTURE_POWER_PIN 13
#define FIXTURE_POWER_ON HIGH
#define FIXTURE_POWER_OFF LOW
#define SIGNAL_DIR_PIN 5
#define SIGNAL_DIR_INPUT LOW
#define SIGNAL_DIR_OUTPUT HIGH

void programmer_view_create(FactoryTest* ft);
void programmer_view_update();

void setup()
{
    ft.init();
    pinMode(GPIO_NUM_15, OUTPUT); // motor
    programmer_view_create(&ft);
}

void loop() { programmer_view_update(); }
