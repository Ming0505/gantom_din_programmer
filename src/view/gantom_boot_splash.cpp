#include "gantom_boot_splash.h"
#include <Arduino.h>

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
        // Need to read encoder state without beeping
        // _ft->_check_encoder(false); // This might be necessary depending on FactoryTest impl.
        int current_pos = _ft->_enc.getPosition(); 
        if (current_pos != last_pos) {
            _ft->_tone(3500, 20); // Different beep for encoder
            exit_splash = true;   // Exit on encoder movement
            last_pos = current_pos; // Update last known position
        }
        
        // Brief delay to prevent hogging CPU and allow other tasks
        delay(10); 
    }
    // Clear screen after exiting splash (optional, depends on desired transition)
    // _ft->_canvas->fillScreen(TFT_BLACK);
    // _ft->_canvas_update();
}

// Private helper function to draw the Gantom logo and version info
void GantomBootSplash::_drawSplash()
{
    // Ensure canvas is ready (might be handled by FactoryTest::init)
    if (!_ft || !_ft->_canvas) return; // Safety check

    _ft->_canvas->startWrite();
    _ft->_canvas->fillScreen(TFT_BLACK);  // Black background
    
    // Calculate center coordinates for easier positioning
    int centerX = _ft->_canvas->width() / 2;
    int centerY = _ft->_canvas->height() / 2;

    // --- Draw Gantom Logo in DMX-horizontal style ---
    // Draw a white rounded rectangle as the logo background
    int rect_w = 180; // width of the logo box
    int rect_h = 60;  // height of the logo box
    int rect_r = 12;  // corner radius
    int rect_y = centerY - rect_h / 2 - 10; // slightly above center
    _ft->_canvas->fillRoundRect(centerX - rect_w/2, rect_y, rect_w, rect_h, rect_r, TFT_WHITE);

    // Draw 'GANTOM' text, centered in the rectangle
    _ft->_canvas->setTextColor(TFT_BLACK);  // Black text on white background
    _ft->_canvas->setFont(&fonts::Font2); // Use a bold/large font
    _ft->_canvas->setTextSize(2);
    _ft->_canvas->drawCenterString("GANTOM", centerX, rect_y + 16);

    // Draw 'DMX TOOL' text below 'GANTOM'
    _ft->_canvas->setTextSize(1);
    _ft->_canvas->drawCenterString("DMX TOOL", centerX, rect_y + 40);

    // --- Draw Version Info ---
    _ft->_canvas->setTextColor(TFT_WHITE);  // White text on black background
    _ft->_canvas->setFont(&fonts::Font0); // Smallest font
    _ft->_canvas->setTextSize(1);
    _ft->_canvas->drawCenterString(FW_VERISON, centerX, rect_y + rect_h + 18); // below the logo box

    // Finish drawing and push to display
    _ft->_canvas->endWrite();
    _ft->_canvas_update(); 
} 