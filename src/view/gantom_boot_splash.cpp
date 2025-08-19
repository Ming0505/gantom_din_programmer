// src/view/gantom_boot_splash.cpp
#include "gantom_boot_splash.h"
#include <Arduino.h>

GantomBootSplash::GantomBootSplash(FactoryTest* ft)
  : _ft(ft)
{
    // Nothing special to initialize
}

void GantomBootSplash::show(uint32_t timeout_ms)
{
    _drawSplash();
    
    uint32_t start_time = millis();
    bool exit_splash = false;
    
    // Wait for timeout or user interaction
    while (!exit_splash) {
        // Check for timeout
        if (millis() - start_time > timeout_ms) {
            exit_splash = true;
        }
        
        // Check for button press
        if (!_ft->_btn_pwr.read()) {
            _ft->_tone(3000, 50);
            exit_splash = true;
        }
        
        // Check for encoder movement
        _ft->_check_encoder(false);
        static int last_pos = 0;
        int current_pos = _ft->_enc.getPosition();
        if (current_pos != last_pos) {
            _ft->_tone(3500, 20);
            exit_splash = true;
            last_pos = current_pos;
        }
        
        delay(10); // Short delay to prevent CPU hogging
    }
}

void GantomBootSplash::_drawSplash()
{
    _ft->_canvas->fillScreen(0x0000);  // Black background
    
    // Draw Gantom logo - simplified version
    int centerX = _ft->_canvas->width() / 2;
    int centerY = _ft->_canvas->height() / 2;
    
    // Draw logo outline
    _ft->_canvas->fillRoundRect(centerX - 70, centerY - 40, 160, 80, 10, 0xFFFF);
    
    // Draw text
    _ft->_canvas->setTextColor(0x0000);  // Black text on white logo
    _ft->_canvas->setFont(&fonts::efontCN_24_b);
    _ft->_canvas->setTextSize(2);
    _ft->_canvas->drawCenterString("GANTOM", centerX+10, centerY - 30);
    _ft->_canvas->setFont(&fonts::efontCN_24);
    _ft->_canvas->setTextSize(1);
    _ft->_canvas->drawCenterString("DMX TOOL", centerX+10, centerY + 15);
    
    // Version info
    _ft->_canvas->setTextColor(0xFFFF);  // White text on black background
    _ft->_canvas->setTextSize(1);
    _ft->_canvas->drawCenterString("v1.0", centerX+10, centerY + 40);

    _ft->_canvas_update();
} 