/**
 * @file main.cpp
 * @author Forairaaaaa / Modified by AI
 * @brief Main application entry point for DMX Tool
 * @version 0.2
 * @date 2024-04-02 // Update date
 *
 * @copyright Copyright (c) 2023
 *
 */
#include "factory_test/factory_test.h" // Hardware abstraction
#include "view/gantom_boot_splash.h" // Boot splash screen
#include "menu/MenuSystem.h"        // The new menu system
#include <Preferences.h>            // For saving settings

// Static instances of core components
static FactoryTest ft;
static Preferences preferences; // Note: Keep scope broad enough for MenuSystem
static GantomBootSplash splash(&ft);
static MenuSystem menu(&ft, &preferences); // Pass FactoryTest and Preferences

// Remove old view function declarations
// void view_create(FactoryTest* ft);
// void view_update();

void setup()
{
    // Initialize hardware via FactoryTest
    ft.init();

    // Show the boot splash screen for a defined duration
    splash.show(2000); // Show splash for 2000 ms (2 seconds)

    // Initialize the menu system (loads settings)
    menu.init(); 

    // No longer call view_create
    // view_create(&ft);
}

void loop() {
    // Continuously update the menu system
    menu.update(); 
    // No longer call view_update
    // view_update(); 
}
