#include "gantom_boot_splash.h"
#include "../factory_test/factory_test.h"
#include <Arduino.h>
#include "gantom_logo.h"
#include "gantom_text_logo.h"

// Constructor: Initializes the pointer to FactoryTest
GantomBootSplash::GantomBootSplash(FactoryTest* ft)
  : _ft(ft)
{
    // Nothing special to initialize here, pointer is assigned in initializer list
}

// Function to show the splash screen and wait for timeout or user interaction
void GantomBootSplash::show(uint32_t timeout_ms)
{
    // First, draw the static splash screen content
    _drawSplash();
    
    // Record the start time
    uint32_t start_time = millis();
    bool exit_splash = false;
    
    // Main loop: Wait until timeout expires or user interacts
    while (!exit_splash) {
        // Check 1: Timeout condition
        if (millis() - start_time > timeout_ms) {
            exit_splash = true; // Exit if time is up
        }
        
        // Check 2: Power button press (active low)
        if (!_ft->_btn_pwr.read()) {
            _ft->_tone(3000, 50); // Short beep on interaction
            exit_splash = true;   // Exit on button press
            while(!_ft->_btn_pwr.read()) delay(10); // Wait for button release
        }
        
        // Check 3: Encoder movement
        // Get current encoder position (relative handling)
        static int last_pos = 0;
        int current_pos = _ft->_enc.getPosition(); 
        if (current_pos != last_pos) {
            _ft->_tone(3500, 20); // Different beep for encoder
            exit_splash = true;   // Exit on encoder movement
            last_pos = current_pos; // Update last known position
        }
        
        // Brief delay to prevent hogging CPU and allow other tasks
        delay(10); 
    }
}

// Private helper function to draw the Gantom logo and version info
void GantomBootSplash::_drawSplash()
{
    // Clear the screen
    _ft->_canvas->fillScreen(TFT_BLACK);
    
    // Calculate center position
    int16_t centerX = _ft->_canvas->width() / 2;
    int16_t centerY = _ft->_canvas->height() / 2;
    
    // Draw the Gantom logo (120x30 pixels, RGB565 format)
    _ft->_canvas->pushImage(centerX - 60, centerY - 15, 120, 30, (uint16_t*)gantomLogo120_30);
    
    // Draw version info below the logo
    _ft->_canvas->setTextColor(TFT_WHITE);
    _ft->_canvas->setTextSize(1);
    _ft->_canvas->setCursor(centerX - 20, centerY + 20);
    _ft->_canvas->print("v1.0.0");
} 