#include "MenuSystem.h"
#include <Arduino.h>

// Constants
const char* PREF_NAMESPACE = "dmx_tool"; // Namespace for Preferences
const char* PREF_KEY_DMX_ADDR = "dmxAddr";
const char* PREF_KEY_LOOK_CH_PREFIX = "lookCh"; // e.g., lookCh0, lookCh1
const char* PREF_KEY_FLASH_BOOT = "flashBoot";
const uint32_t CONFIRM_MSG_DURATION_MS = 2000; // How long confirmation messages stay

// Constructor Implementation
MenuSystem::MenuSystem(FactoryTest* ft, Preferences* prefs)
    : _ft(ft), _preferences(prefs), _currentState(MAIN_MENU), 
      _dmxAddress(1), _flashBootEnabled(false), 
      _mainMenuSelection(0), _staticLookSelectedChannel(0), _flashBootSelection(0), 
      _confirmationTimer(0), _powerOffTimer(0), _buttonPressTime(0), _buttonHeldState(false),
      _staticLookScrollOffset(0), _isEditingStaticLookValue(false),
      _lastEncoderTime(0), _recentEncoderDiffSum(0), _encoderStepCounter(0) // Initialize encoder accel vars
{
    // Initialize static looks array to 0
    for (int i = 0; i < 8; ++i) {
        _staticLooks[i] = 0;
    }
}

// Initialize the menu system
void MenuSystem::init() {
    // Attempt to open Preferences with the defined namespace
    if (!_preferences->begin(PREF_NAMESPACE, false)) {
        Serial.println("Error opening preferences!");
        // Handle error? Maybe load defaults or halt?
        // For now, we proceed with defaults if loading fails.
    }
    // Load settings from Preferences
    _loadSettings();
    // Ensure initial selection is valid (though constructor defaults should be fine)
    _mainMenuSelection = 0;
    _currentState = MAIN_MENU; 
    // Don't close preferences here, keep it open for saving
    // _preferences->end(); 
}

// Main update loop: Handle input and draw screen based on state
void MenuSystem::update() {
    _handleInput(); // Process button/encoder inputs
    _drawScreen();  // Draw the current screen based on state
}

// --- Helper Functions ---

// Load all settings from Preferences storage
void MenuSystem::_loadSettings() {
    // Load DMX Address, default to 1 if not found
    _dmxAddress = _preferences->getUInt(PREF_KEY_DMX_ADDR, 1);
    // Clamp address to valid range (1-512)
    if (_dmxAddress == 0 || _dmxAddress > 512) {
        _dmxAddress = 1;
    }

    // Load Static Look channels
    for (int i = 0; i < 8; ++i) {
        char key[10]; // Buffer for key name e.g., "lookCh0"
        snprintf(key, sizeof(key), "%s%d", PREF_KEY_LOOK_CH_PREFIX, i);
        // Load channel value, default to 0 if not found
        _staticLooks[i] = _preferences->getUChar(key, 0);
    }

    // Load Flash Boot setting, default to false if not found
    _flashBootEnabled = _preferences->getBool(PREF_KEY_FLASH_BOOT, false);

    Serial.printf("Settings loaded: Addr=%d, FlashBoot=%d\n", _dmxAddress, _flashBootEnabled);
}

// Save DMX Address to Preferences
void MenuSystem::_saveDmxAddress() {
    _preferences->putUInt(PREF_KEY_DMX_ADDR, _dmxAddress);
    Serial.printf("Saved DMX Address: %d\n", _dmxAddress);
}

// Save a specific Static Look channel value to Preferences
void MenuSystem::_saveStaticLookChannel(int channelIndex) {
    if (channelIndex >= 0 && channelIndex < 8) {
        char key[10];
        snprintf(key, sizeof(key), "%s%d", PREF_KEY_LOOK_CH_PREFIX, channelIndex);
        _preferences->putUChar(key, _staticLooks[channelIndex]);
        Serial.printf("Saved Static Look Ch%d: %d\n", channelIndex + 1, _staticLooks[channelIndex]);
    }
}

// Save Flash Boot setting to Preferences
void MenuSystem::_saveFlashBootSetting() {
    _preferences->putBool(PREF_KEY_FLASH_BOOT, _flashBootEnabled);
    Serial.printf("Saved Flash Boot Setting: %s\n", _flashBootEnabled ? "On" : "Off");
}

// Calculate percentage (0-100) from a 0-255 value
uint8_t MenuSystem::_calculatePercentage(uint8_t value255) {
    // Use floating point for intermediate calculation for accuracy
    return (uint8_t)round(((float)value255 / 255.0f) * 100.0f);
}

// --- Acceleration Helper ---
// Calculates an accelerated step based on time between encoder events
int MenuSystem::_calculateAcceleratedDiff(int raw_diff) {
    if (raw_diff == 0) return 0; 

    uint32_t currentTime = millis();
    uint32_t timeDiff = currentTime - _lastEncoderTime; 
    _lastEncoderTime = currentTime;

    // --- Define Acceleration Curve Parameters ---
    const uint32_t MAX_TIME_MS = 180; // Increased threshold for slowest speed (was 150)
    const uint32_t MIN_TIME_MS = 20;  
    const int MIN_MULT = 1;           
    const int MAX_MULT = 30;          // Keep top speed high

    int multiplier = MIN_MULT;

    // Remove special case for timeDiff == 0
    /*if (timeDiff == 0) {
        multiplier = 5; 
    } else*/ if (timeDiff < MIN_TIME_MS) { // Fastest speed
        multiplier = MAX_MULT;
    } else if (timeDiff < MAX_TIME_MS) { // Interpolation zone
        float factor = (float)(MAX_TIME_MS - timeDiff) / (float)(MAX_TIME_MS - MIN_TIME_MS);
        // Increase exponent for a flatter start and steeper end
        factor = pow(factor, 3.0f); // Was 2.5f
        multiplier = MIN_MULT + (int)(factor * (MAX_MULT - MIN_MULT));
    } 
    // else timeDiff >= MAX_TIME_MS, multiplier remains MIN_MULT (1)
    
    if (multiplier < MIN_MULT) multiplier = MIN_MULT; 

    int acceleratedStep = ((raw_diff > 0) ? 1 : -1) * multiplier;

    Serial.printf(" Accel: raw=%d, time=%lu, mult=%d, step=%d\n", 
                  raw_diff, timeDiff, multiplier, acceleratedStep);

    return acceleratedStep;
}

// --- Input Handling Logic ---
void MenuSystem::_handleInput() {
    // Constants for long press detection
    const uint32_t LONG_PRESS_DURATION_MS = 700;

    // --- Encoder Handling --- 
    static int last_enc_pos = 0; // Store position from previous update
    int enc_diff_raw = 0; // Raw difference from hardware
    bool encoder_changed = _ft->_check_encoder(false); // Check if hardware state changed

    if (encoder_changed) {
        int current_pos = _ft->_enc.getPosition(); // Get current absolute position
        enc_diff_raw = current_pos - last_enc_pos;
        last_enc_pos = current_pos; // Update for next time
        Serial.printf("Encoder Changed: pos=%d, raw_diff=%d\n", current_pos, enc_diff_raw);
    } else {
        enc_diff_raw = 0; // Ensure diff is 0 if encoder hardware state didn't change
    }

    // Calculate accelerated difference
    int enc_diff = _calculateAcceleratedDiff(enc_diff_raw);

    // --- Button Handling --- 
    // Read current button state (active low)
    bool isButtonPressed = !_ft->_btn_pwr.read(); 
    uint32_t currentTime = millis();

    // Button Press/Hold/Release Logic
    bool short_pressed = false;
    bool long_pressed = false;

    // Check for button press start
    if (isButtonPressed && _buttonPressTime == 0) { 
        _buttonPressTime = currentTime; // Record press time
        _buttonHeldState = false; // Reset held state
    } 
    // Check for button held long enough
    else if (isButtonPressed && _buttonPressTime != 0 && !_buttonHeldState) {
        if (currentTime - _buttonPressTime >= LONG_PRESS_DURATION_MS) {
            long_pressed = true;
            _buttonHeldState = true; // Mark as held to prevent repeated triggering
             _ft->_tone(2500, 100); // Different beep for long press
            // Reset press time to prevent short press on release after long press
            // _buttonPressTime = 0; 
        }
    } 
    // Check for button release
    else if (!isButtonPressed && _buttonPressTime != 0) {
        // Only trigger short press if it wasn't already handled as a long press
        if (!_buttonHeldState) {
            short_pressed = true;
            _ft->_tone(3000, 50); 
            Serial.println("Input: Short Press Detected");
        }
        // Reset button timing state on release
        _buttonPressTime = 0;
        _buttonHeldState = false;
    }

    // Add else if for long press debug
    else if (long_pressed) { 
        Serial.println("Input: Long Press Detected");
    }

    // --- State-Specific Input Handling ---
    // Only call handlers if there actually was input
    if (enc_diff != 0 || short_pressed || long_pressed) {
        switch (_currentState) {
            case MAIN_MENU:                    
                Serial.printf(" -> MainMenu Input: diff=%d, short=%d, long=%d\n", enc_diff, short_pressed, long_pressed);
                _handleInputMainMenu(enc_diff, short_pressed, long_pressed); 
                break;
            case SET_DMX_ADDRESS:              
                 Serial.printf(" -> SetAddress Input: diff=%d, short=%d, long=%d\n", enc_diff, short_pressed, long_pressed);
                _handleInputSetAddress(enc_diff, short_pressed, long_pressed); 
                break;
            case SET_STATIC_LOOK:              
                 Serial.printf(" -> StaticLook Input: diff=%d, short=%d, long=%d, editing=%d\n", enc_diff, short_pressed, long_pressed, _isEditingStaticLookValue);
                _handleInputSetStaticLook(enc_diff, short_pressed, long_pressed); 
                break;
            case TOGGLE_FLASH_BOOT_SELECT:     
                 Serial.printf(" -> FlashBootSelect Input: diff=%d, short=%d, long=%d\n", enc_diff, short_pressed, long_pressed);
                _handleInputToggleFlashBootSelect(enc_diff, short_pressed, long_pressed); 
                break;
            case TOGGLE_FLASH_BOOT_CONFIRM_ON:
            case TOGGLE_FLASH_BOOT_CONFIRM_OFF: 
                 Serial.printf(" -> FlashBootConfirm Input: short=%d, long=%d\n", short_pressed, long_pressed);
                _handleInputToggleFlashBootConfirm(short_pressed, long_pressed); 
                break; 
            case ARKANOID_GAME:                
                Serial.printf(" -> Arkanoid Input: diff=%d, short=%d, long=%d\n", enc_diff, short_pressed, long_pressed);
                _handleInputArkanoid(short_pressed, long_pressed); 
                break;
            case POWERING_OFF:                 
                // No input handled here
                break; 
        }
    } else if (_currentState == POWERING_OFF) {
        // Still need to check power off timer even if no input
        if (millis() - _powerOffTimer > 1000) { 
            _ft->_power_off(); 
        }
    }
}

// --- Update State-Specific Handlers for Long Press ---

// Handle input specifically for the Main Menu
void MenuSystem::_handleInputMainMenu(int enc_diff, bool short_pressed, bool long_pressed) {
    // Long press has no action on main menu
    if (long_pressed) return; 

    if (enc_diff != 0) {
        _mainMenuSelection += (enc_diff > 0) ? 1 : -1;
        if (_mainMenuSelection < 0) _mainMenuSelection = NUM_MAIN_MENU_ITEMS - 1;
        else if (_mainMenuSelection >= NUM_MAIN_MENU_ITEMS) _mainMenuSelection = 0;
        _ft->_tone(3500, 20);
    }

    if (short_pressed) {
        switch (_mainMenuSelection) {
            case 0: // Set DMX Address
                _currentState = SET_DMX_ADDRESS;
                break;
            case 1: // Set Static Look
                _currentState = SET_STATIC_LOOK;
                _staticLookSelectedChannel = 0; // Start selection at Channel 1
                _staticLookScrollOffset = 0;    // Start scroll at the top
                _isEditingStaticLookValue = false; // Start in channel selection mode
                break;
            case 2: // Toggle Flash Boot
                _currentState = TOGGLE_FLASH_BOOT_SELECT;
                _flashBootSelection = _flashBootEnabled ? 1 : 0;
                break;
            case 3: // Arkanoid Game
                 _currentState = ARKANOID_GAME;
                 _ft->_arkanoid_setup(); // Initialize the game
                 break;
            case 4: // Power Off
                _currentState = POWERING_OFF;
                _powerOffTimer = millis();
                break;
        }
    }
}

// Handle input for the Set DMX Address screen
void MenuSystem::_handleInputSetAddress(int enc_diff, bool short_pressed, bool long_pressed) {
    // Long press goes back to Main Menu without saving
    if (long_pressed) {
        _currentState = MAIN_MENU;
        _mainMenuSelection = 0; // Highlight the address item again
        // Reload original value? Or keep potentially unsaved change?
        // Let's reload for consistency
        _loadSettings(); // Reloads all settings, including DMX address
        return;
    }

    if (enc_diff != 0) {
        int newAddress = _dmxAddress + enc_diff; 
        if (newAddress < 1) newAddress = 1;
        else if (newAddress > 512) newAddress = 512;
        _dmxAddress = (uint16_t)newAddress;
        _ft->_tone(3800, 15); 
    }

    // Short press saves and returns to main menu
    if (short_pressed) {
        _saveDmxAddress();
        _currentState = MAIN_MENU;
        _mainMenuSelection = 0; 
    }
}

// Handle input for the combined Static Look screen with bars (4x2 grid)
void MenuSystem::_handleInputSetStaticLook(int enc_diff, bool short_pressed, bool long_pressed) {
    // Long press: Go back to main menu, saving current channel if editing
    if (long_pressed) {
        if (_isEditingStaticLookValue) {
            _saveStaticLookChannel(_staticLookSelectedChannel);
            _isEditingStaticLookValue = false; 
        }
        _currentState = MAIN_MENU;
        _mainMenuSelection = 1; 
        return;
    }

    // Short press: Toggle between selecting channel and editing value
    if (short_pressed) {
        _isEditingStaticLookValue = !_isEditingStaticLookValue;
        if (!_isEditingStaticLookValue) { 
            _saveStaticLookChannel(_staticLookSelectedChannel);
        }
         _ft->_tone(_isEditingStaticLookValue ? 2800 : 3200, 50); 
        return; 
    }

    // Encoder difference
    if (enc_diff != 0) {
        if (_isEditingStaticLookValue) {
            // --- Editing Mode --- (Use accelerated diff)
            int currentValue = _staticLooks[_staticLookSelectedChannel];
            int newValue = currentValue + enc_diff; // Apply accelerated step
            if (newValue < 0) newValue = 0;
            else if (newValue > 255) newValue = 255;
            _staticLooks[_staticLookSelectedChannel] = (uint8_t)newValue;
            _ft->_tone(3800, 15); 
            Serial.printf("    Edit Value: Ch=%d, Old=%d, New=%d (raw_diff=%d)\n", 
                          _staticLookSelectedChannel + 1, currentValue, newValue, enc_diff); // Debug raw diff too?
        } else {
            // --- Channel Selection Mode --- (No acceleration needed, use raw diff)
            // Use raw_diff directly for simple +/- 1 navigation
            int raw_diff_nav = (enc_diff > 0) ? 1 : ((enc_diff < 0) ? -1 : 0);
            if (raw_diff_nav != 0) { 
                int prevChannel = _staticLookSelectedChannel;
                _staticLookSelectedChannel += raw_diff_nav; 
                if (_staticLookSelectedChannel < 0) _staticLookSelectedChannel = 7;
                else if (_staticLookSelectedChannel > 7) _staticLookSelectedChannel = 0;
                _ft->_tone(3500, 20); 
                Serial.printf("    Select Ch: Old=%d, New=%d\n", prevChannel + 1, _staticLookSelectedChannel + 1);
            }
        }
    }
}

// Handle input for the Flash Boot selection screen (On/Off)
void MenuSystem::_handleInputToggleFlashBootSelect(int enc_diff, bool short_pressed, bool long_pressed) {
    // Long press goes back to Main Menu without changing
    if (long_pressed) {
        _currentState = MAIN_MENU;
        _mainMenuSelection = 2; // Highlight the flash boot item
        return;
    }
    
    if (enc_diff != 0) {
        _flashBootSelection = 1 - _flashBootSelection; 
        _ft->_tone(3500, 20);
    }

    // Short press confirms selection
    if (short_pressed) {
        _flashBootEnabled = (_flashBootSelection == 1);
        _saveFlashBootSetting();
        if (_flashBootEnabled) _currentState = TOGGLE_FLASH_BOOT_CONFIRM_ON;
        else _currentState = TOGGLE_FLASH_BOOT_CONFIRM_OFF;
        _confirmationTimer = millis(); 
    }
}

// Handle input for the Flash Boot confirmation screen
void MenuSystem::_handleInputToggleFlashBootConfirm(bool short_pressed, bool long_pressed) {
    // Long press returns immediately
    if (long_pressed) {
         _currentState = MAIN_MENU; 
         _mainMenuSelection = 2;
         return;
    }
    // Short press returns immediately
    if (short_pressed) {
         _currentState = MAIN_MENU; 
         _mainMenuSelection = 2;
         return;
    }
    // Timeout returns
    if (millis() - _confirmationTimer > CONFIRM_MSG_DURATION_MS) {
        _currentState = MAIN_MENU; 
        _mainMenuSelection = 2; 
    }
}

// Handle input for Arkanoid game (only exit on long press)
void MenuSystem::_handleInputArkanoid(bool short_pressed, bool long_pressed) {
    // Call encoder check in case the game loop expects it for input
    _ft->_check_encoder(false); // false = don't beep here

    // Short press and normal encoder diff are handled by the game loop itself (_ft->_arkanoid_loop)
    if (long_pressed) {
        // Long press exits the game
        _ft->_noTone(); // Stop any game sounds potentially stuck
        // Optionally unload game resources if _arkanoid_unload exists?
        // _ft->_arkanoid_unload(); // Assume this doesn't exist based on header
        _currentState = MAIN_MENU;
        _mainMenuSelection = 3; // Highlight Arkanoid item
    }
    // IMPORTANT: The game loop needs to be called frequently. 
    // We will call it from _drawArkanoid.
}

// --- Drawing Functions ---

// Helper function to draw the standard header
void MenuSystem::_drawHeader(const char* title) {
    int headerHeight = 18; 
    _ft->_canvas->fillRect(0, 0, _ft->_canvas->width(), headerHeight, TFT_DARKGREY); 
    _ft->_canvas->drawRect(0, 0, _ft->_canvas->width(), headerHeight, TFT_WHITE);   
    _ft->_canvas->setTextColor(TFT_WHITE, TFT_DARKGREY); 
    _ft->_canvas->setFont(&fonts::Font2);
    _ft->_canvas->setTextSize(1);
    // Adjust Y position for better vertical centering
    int textY = (headerHeight - _ft->_canvas->fontHeight()) / 2;
    _ft->_canvas->drawCenterString(title, _ft->_canvas->width() / 2, textY);
}

// Helper function to draw the standard footer with hint text
void MenuSystem::_drawFooter(const char* hint) {
    int footerHeight = 18; // << Increased height slightly
    int footerY = _ft->_canvas->height() - footerHeight;
    _ft->_canvas->fillRect(0, footerY, _ft->_canvas->width(), footerHeight, TFT_DARKGREY); 
    _ft->_canvas->drawRect(0, footerY, _ft->_canvas->width(), footerHeight, TFT_WHITE);   
    _ft->_canvas->setTextColor(TFT_WHITE, TFT_DARKGREY); 
    _ft->_canvas->setFont(&fonts::Font2); 
    _ft->_canvas->setTextSize(1);
    int textY = footerY + (footerHeight - _ft->_canvas->fontHeight()) / 2; // Recalculate Y based on new height
    _ft->_canvas->drawCenterString(hint, _ft->_canvas->width() / 2, textY);
}

void MenuSystem::_drawScreen() {
    // Only clear screen if NOT in Arkanoid game state
    // Arkanoid loop should handle its own drawing fully
    if (_currentState != ARKANOID_GAME) {
        _ft->_canvas->startWrite(); // Begin transaction for faster drawing (if supported)
        _ft->_canvas->fillScreen(TFT_BLACK); // Clear screen
    }

    switch (_currentState) {
        case MAIN_MENU:                    _drawMainMenu(); break;
        case SET_DMX_ADDRESS:              _drawSetAddress(); break;
        case SET_STATIC_LOOK:              _drawSetStaticLook(); break;
        case TOGGLE_FLASH_BOOT_SELECT:     _drawToggleFlashBootSelect(); break;
        case TOGGLE_FLASH_BOOT_CONFIRM_ON: _drawToggleFlashBootConfirm(true); break;
        case TOGGLE_FLASH_BOOT_CONFIRM_OFF:_drawToggleFlashBootConfirm(false); break;
        case ARKANOID_GAME:                _drawArkanoid(); break;
        case POWERING_OFF:                 _drawPoweringOff(); break;
    }
    
    // Only end write if we started one
    if (_currentState != ARKANOID_GAME) {
        _ft->_canvas->endWrite(); // End transaction
    }
    // Always push buffer to screen (Arkanoid might draw directly or use canvas)
    _ft->_canvas_update(); // Push buffer to screen
}

// Draw the Main Menu screen
void MenuSystem::_drawMainMenu() {
    _drawHeader("Main Menu"); 
    
    const char* menuItems[] = {
        "Set DMX Address", "Set Static Look", "Toggle Flash Boot", "Arkanoid", "Power Off"
    };
    const int PADDING_LEFT = 5; 
    int headerHeight = 18;
    int footerHeight = 15;
    int contentHeight = _ft->_canvas->height() - headerHeight - footerHeight;
    int lineHeight = contentHeight / NUM_MAIN_MENU_ITEMS; // Distribute lines in available space
    int yPos = headerHeight + (lineHeight - _ft->_canvas->fontHeight(&fonts::Font2)) / 2; // Center first item vertically in its slot
    if (lineHeight < _ft->_canvas->fontHeight(&fonts::Font2)) lineHeight = _ft->_canvas->fontHeight(&fonts::Font2) + 2; // Prevent overlap

    // Set text properties for items
    _ft->_canvas->setTextColor(TFT_WHITE, TFT_BLACK); // Text on black background within content area
    _ft->_canvas->setFont(&fonts::Font2);
    _ft->_canvas->setTextSize(1);
    
    uint16_t highlightBgColor = TFT_BLUE; // Use Blue for highlight
    uint16_t highlightTextColor = TFT_WHITE;

    // Draw each menu item within the content area
    for (int i = 0; i < NUM_MAIN_MENU_ITEMS; ++i) {
        int currentY = headerHeight + i * lineHeight + (lineHeight - _ft->_canvas->fontHeight(&fonts::Font2))/2 ;
        if (i == _mainMenuSelection) {
            _ft->_canvas->fillRect(0, headerHeight + i * lineHeight, _ft->_canvas->width(), lineHeight, highlightBgColor); // Highlight BG
            _ft->_canvas->setTextColor(highlightTextColor); // Highlight text
            _ft->_canvas->drawString(menuItems[i], PADDING_LEFT, currentY);
            _ft->_canvas->setTextColor(TFT_WHITE, TFT_BLACK); // Reset color
        } else {
            _ft->_canvas->setTextColor(TFT_WHITE, TFT_BLACK); // Normal text
            _ft->_canvas->drawString(menuItems[i], PADDING_LEFT, currentY);
        }
    }

    _drawFooter(""); // No hint needed for main menu
}

// Draw the Set DMX Address screen
void MenuSystem::_drawSetAddress() {
    _drawHeader("Set DMX Address");

    // Address Value - Large Font, Centered in content area
    int headerHeight = 18;
    int footerHeight = 15;
    int contentCenterY = headerHeight + (_ft->_canvas->height() - headerHeight - footerHeight) / 2;

    _ft->_canvas->setFont(&fonts::Font6); 
    _ft->_canvas->setTextSize(1);
    _ft->_canvas->setTextColor(TFT_WHITE, TFT_BLACK);
    char addrStr[4]; 
    snprintf(addrStr, sizeof(addrStr), "%03u", _dmxAddress);
    int textWidth = _ft->_canvas->textWidth(addrStr); 
    int textHeight = _ft->_canvas->fontHeight();
    _ft->_canvas->drawString(addrStr, (_ft->_canvas->width() - textWidth) / 2, contentCenterY - textHeight / 2);

    _drawFooter("Turn=Change | Press=Save | Hold=Back");
}

// Draw the screen for toggling the Flash Boot setting
void MenuSystem::_drawToggleFlashBootSelect() {
    _drawHeader("Toggle Flash Boot");

    const char* options[] = {"Off", "On"};
    int headerHeight = 18;
    int footerHeight = 15;
    int contentHeight = _ft->_canvas->height() - headerHeight - footerHeight;
    int lineHeight = 25; // Fixed height for options
    int startY = headerHeight + (contentHeight - lineHeight * 2) / 2; // Center the two options vertically

    _ft->_canvas->setFont(&fonts::Font2);
    _ft->_canvas->setTextSize(1);

    uint16_t highlightBgColor = TFT_BLUE; // Use Blue for highlight
    uint16_t highlightTextColor = TFT_WHITE;

    for (int i = 0; i < 2; ++i) {
        int currentY = startY + i * lineHeight;
        int textY = currentY + (lineHeight - _ft->_canvas->fontHeight())/2;
        if (i == _flashBootSelection) {
            _ft->_canvas->fillRect(0, currentY, _ft->_canvas->width(), lineHeight, highlightBgColor); // Highlight BG
             _ft->_canvas->setTextColor(highlightTextColor); // Highlight text
            _ft->_canvas->drawCenterString(options[i], _ft->_canvas->width() / 2, textY);
            _ft->_canvas->setTextColor(TFT_WHITE, TFT_BLACK); 
        } else {
             _ft->_canvas->setTextColor(TFT_WHITE, TFT_BLACK); // Normal text
             _ft->_canvas->drawCenterString(options[i], _ft->_canvas->width() / 2, textY);
        }
    }

    _drawFooter("Turn=Select | Press=Confirm | Hold=Back");
}

// Draw the screen for confirming the Flash Boot setting
void MenuSystem::_drawToggleFlashBootConfirm(bool isConfirmingOn) {
     _drawHeader("Flash Boot Setting");

    // Confirmation Message centered in content area
    int headerHeight = 18;
    int footerHeight = 15; // Footer height even if no text shown
    int contentCenterY = headerHeight + (_ft->_canvas->height() - headerHeight - footerHeight) / 2;

    const char* msg1 = isConfirmingOn ? "Power cycling -" : "Power cycling -";
    const char* msg2 = isConfirmingOn ? "Fixture should flash." : "Fixture should NOT flash.";

    _ft->_canvas->setFont(&fonts::Font2); 
    _ft->_canvas->setTextSize(1);
    _ft->_canvas->setTextColor(TFT_WHITE, TFT_BLACK);
    _ft->_canvas->drawCenterString(msg1, _ft->_canvas->width() / 2, contentCenterY - 10);
    _ft->_canvas->drawCenterString(msg2, _ft->_canvas->width() / 2, contentCenterY + 10);

     _drawFooter("(Press/Hold to continue)"); // Hint for confirmation
}

// Draw the Powering Off screen
void MenuSystem::_drawPoweringOff() {
    _drawHeader("Power Off");

    // Message centered in content area
    int headerHeight = 18;
    int footerHeight = 15;
    int contentCenterY = headerHeight + (_ft->_canvas->height() - headerHeight - footerHeight) / 2;

    _ft->_canvas->setTextColor(TFT_WHITE, TFT_BLACK);
    _ft->_canvas->setFont(&fonts::Font4);
    _ft->_canvas->setTextSize(1);
    _ft->_canvas->drawCenterString("Powering Off...", _ft->_canvas->width() / 2, contentCenterY - _ft->_canvas->fontHeight()/2);

     _drawFooter(""); // No hint needed
}

// Draw the combined Static Look screen with bars (4x2 grid)
void MenuSystem::_drawSetStaticLook() {
    _drawHeader("Set Static Look");

    const int COLS = 2;
    const int ROWS = 4;
    int headerHeight = 18;
    int footerHeight = 18; 
    int contentWidth = _ft->_canvas->width();
    int contentHeight = _ft->_canvas->height() - headerHeight - footerHeight; 
    
    int horizontalPadding = 8; 
    int colWidth = (contentWidth - horizontalPadding) / COLS;
    
    // Calculate needed height per item: Text height + Bar height + Padding
    int fontHeight = _ft->_canvas->fontHeight(&fonts::Font2); // Get height of Font2
    int bottomBarHeight = 2;
    int verticalPadding = 4; // Padding between rows (below the bar)
    int totalItemHeight = fontHeight + bottomBarHeight + verticalPadding + 2; // Add 2px buffer above text
    int barWidth = colWidth - 10; 

    // Calculate the total height needed for the grid
    int gridTotalHeight = totalItemHeight * ROWS - verticalPadding; // No padding after last row
    // Calculate starting Y to center the grid vertically
    int gridStartY = headerHeight + (contentHeight - gridTotalHeight) / 2;
    if (gridStartY < headerHeight) gridStartY = headerHeight; 

    // Draw the 8 channels in a 4x2 grid
    for (int row = 0; row < ROWS; ++row) {
        for (int col = 0; col < COLS; ++col) {
            int channelIndex = row + col * ROWS; 
            if (channelIndex >= 8) break; 

            // Calculate position 
            int colStartX = (col == 0) ? 0 : colWidth + horizontalPadding;
            int itemX = colStartX + 5; 
            // Calculate Y position for the *top* of this item's allocated vertical space
            int itemY = gridStartY + row * totalItemHeight; 
            // The height passed to _drawChannelBar is the full allocated height for positioning text/bar
            int itemH = totalItemHeight - verticalPadding; // Height for drawing within this slot
            
            bool isSelected = (channelIndex == _staticLookSelectedChannel);
            
            char label[15];
            snprintf(label, sizeof(label), "CH%d:%3d(%d%%)", 
                     channelIndex + 1, 
                     _staticLooks[channelIndex], 
                     _calculatePercentage(_staticLooks[channelIndex]));

            _drawChannelBar(itemX, itemY, barWidth, itemH, // Pass itemH for vertical centering
                            _staticLooks[channelIndex], label, isSelected);
        }
    }

    // Hint text using the consistent format
    const char* hintText = _isEditingStaticLookValue ? 
                           "Turn=Change | Press=Select | Hold=Back" : // Edit Mode hint
                           "Turn=Select | Press=Edit   | Hold=Back"; // Select Mode hint (added space for alignment)
    _drawFooter(hintText);
}

// Helper function to draw a single channel item with text and dynamic length bottom line indicator
void MenuSystem::_drawChannelBar(int x, int y, int w, int h, uint8_t value, const char* label, bool selected) {
    
    // --- Color Calculation --- 
    uint16_t valueLineColor;
    if (value < 128) {
        uint8_t red = (value * 2); 
        valueLineColor = _ft->_canvas->color565(red, 255, 0); // Green to Yellow
    } else {
        uint8_t green = 255 - ((value - 128) * 2);
        valueLineColor = _ft->_canvas->color565(255, green, 0); // Yellow to Red
    }
    if (value > 0 && value < 5) valueLineColor = _ft->_canvas->color565(10, 50, 0); // Dim Green for low values
    // Don't use Dark Grey for 0 here, let the empty part handle it

    // Other colors
    uint16_t selectionBorderColor = selected ? (_isEditingStaticLookValue ? TFT_RED : TFT_YELLOW) : TFT_BLACK; 
    uint16_t labelColor = TFT_WHITE; 
    uint16_t emptyLineColor = TFT_DARKGREY; 

    // --- Calculations --- 
    int valueWidth = (int)(((float)value / 255.0f) * w);
    if (valueWidth < 0) valueWidth = 0;
    if (valueWidth > w) valueWidth = w;

    // --- Text Component Calculation & Positioning --- 
    _ft->_canvas->setFont(&fonts::Font2); 
    _ft->_canvas->setTextSize(1);
    int labelHeight = _ft->_canvas->fontHeight();
    int bottomBarHeight = 2;
    int labelY = y + (h - labelHeight - bottomBarHeight - 1) / 2; 
    if (labelY < y) labelY = y; 
    int bottomBarY = labelY + labelHeight + 1; 
    if (bottomBarY > y + h - bottomBarHeight) bottomBarY = y + h - bottomBarHeight;

    // 1. Channel Number Part ("CHx:")
    char chPart[5]; // "CH8:"
    // Need channel index - derive from label or pass as arg? Requires change upstream.
    // **Temporary Workaround:** Extract from the original label (less efficient)
    int channelNum = 0;
    sscanf(label, "CH%d:", &channelNum); // Extract channel number
    snprintf(chPart, sizeof(chPart), "CH%d:", channelNum); 
    int chPartX = x + 2; // Left-aligned
    int chPartWidth = _ft->_canvas->textWidth(chPart);

    // 2. Percentage Part ("(Pct%)")
    char pctPart[7]; // "(100%)"
    uint8_t percentage = _calculatePercentage(value);
    snprintf(pctPart, sizeof(pctPart), "(%d%%)", percentage);
    int pctPartWidth = _ft->_canvas->textWidth(pctPart);
    // Right-align percentage part
    int pctPartX = x + w - pctPartWidth - 2; // Right edge - width - padding

    // 3. Value Part ("Val")
    char valPart[4]; // "255"
    snprintf(valPart, sizeof(valPart), "%d", value);
    int valPartWidth = _ft->_canvas->textWidth(valPart);
    // Calculate max width of value ("255") for alignment
    int maxValWidth = _ft->_canvas->textWidth("255"); 
    // Right-align value relative to the start of the percentage part
    int valPartX = pctPartX - maxValWidth - 2; // Position based on max width, add spacing
    // Adjust X for current value width to achieve right-alignment within the maxValWidth space
    valPartX += (maxValWidth - valPartWidth);

    // --- Drawing --- 
    // 1. Draw Outer Selection Border (if selected)
    if (selected) {
      _ft->_canvas->drawRect(x - 2, y - 2, w + 4, h + 4, selectionBorderColor);
    }
    
    // 2. Draw the Value Indicator Bottom Bar (Dynamic Length)
    if (valueWidth > 0) { _ft->_canvas->fillRect(x, bottomBarY, valueWidth, bottomBarHeight, valueLineColor); }
    int emptyWidth = w - valueWidth;
    if (emptyWidth > 0) { _ft->_canvas->fillRect(x + valueWidth, bottomBarY, emptyWidth, bottomBarHeight, emptyLineColor); }

    // 3. Draw the Text Components (White on Black)
    _ft->_canvas->setTextColor(labelColor, TFT_BLACK); 
    // Clear area behind text first? Optional, drawString usually handles background.
    // _ft->_canvas->fillRect(x, labelY, w, labelHeight, TFT_BLACK);
    _ft->_canvas->drawString(chPart, chPartX, labelY); 
    _ft->_canvas->drawString(valPart, valPartX, labelY);
    _ft->_canvas->drawString(pctPart, pctPartX, labelY);
}

// Drawing function for Arkanoid - delegates to the game's loop/draw function
void MenuSystem::_drawArkanoid() {
    // The FactoryTest class handles drawing internally within its loop function.
    // We just need to call it.
    // NOTE: This assumes _arkanoid_loop handles its own screen clearing/drawing.
    // It also likely handles input internally.
    _ft->_arkanoid_loop(); 
} 