#ifndef MENU_SYSTEM_H
#define MENU_SYSTEM_H

// src/menu/MenuSystem.h

#include "../factory_test/factory_test.h" // Include FactoryTest for hardware access
#include <Preferences.h> // Include Preferences for persistent storage
#include <stdint.h>

// Define the different states the menu can be in
enum MenuState {
    MAIN_MENU,
    SET_DMX_ADDRESS,
    SET_STATIC_LOOK,
    TOGGLE_FLASH_BOOT_SELECT,
    TOGGLE_FLASH_BOOT_CONFIRM_ON,
    TOGGLE_FLASH_BOOT_CONFIRM_OFF,
    ARKANOID_GAME,
    POWERING_OFF
};

// Update number of main menu items
const int NUM_MAIN_MENU_ITEMS = 5; // Was 4

class MenuSystem {
public:
    // Constructor: Takes pointers to FactoryTest and Preferences objects
    MenuSystem(FactoryTest* ft, Preferences* prefs);

    // Initialize the menu system (load settings, etc.)
    void init();

    // Update the menu system (read input, update state, draw screen)
    void update();

private:
    // Pointer to the FactoryTest instance for hardware interaction
    FactoryTest* _ft;
    // Pointer to the Preferences instance for saving/loading settings
    Preferences* _preferences;

    // Current state of the menu
    MenuState _currentState;

    // --- Settings Data ---
    uint16_t _dmxAddress;      // DMX Start Address (1-512)
    uint8_t _staticLooks[8]; // Values for 8 channels (0-255)
    bool _flashBootEnabled;  // Flash on boot setting

    // --- Menu Navigation State ---
    int _mainMenuSelection;        // Currently selected item in the main menu
    int _staticLookSelectedChannel; // Index of the currently selected channel (0-7)
    int _staticLookScrollOffset;   // Index of the top-most visible channel (0-4 typically)
    bool _isEditingStaticLookValue; // True if encoder should change value, false if scrolling channels
    int _flashBootSelection;       // 0 for Off, 1 for On in the selection submenu
    uint32_t _confirmationTimer;   // Timer for how long confirmation messages are shown
    uint32_t _powerOffTimer;       // Timer for delay before actual power off

    // --- Input Timing State ---
    uint32_t _buttonPressTime;     // millis() when button was initially pressed
    bool _buttonHeldState;         // Tracks if the button is currently considered 'held'
    // Encoder Acceleration
    uint32_t _lastEncoderTime;     // millis() of the last encoder change event
    int _recentEncoderDiffSum;   // Sum of recent encoder diffs for acceleration calc
    int _encoderStepCounter;     // Counter for recent encoder steps

    // --- Input Handling ---
    void _handleInput(); // Main input dispatch function
    void _handleInputMainMenu(int enc_diff, bool short_pressed, bool long_pressed);
    void _handleInputSetAddress(int enc_diff, bool short_pressed, bool long_pressed);
    void _handleInputSetStaticLook(int enc_diff, bool short_pressed, bool long_pressed);
    void _handleInputToggleFlashBootSelect(int enc_diff, bool short_pressed, bool long_pressed);
    void _handleInputToggleFlashBootConfirm(bool short_pressed, bool long_pressed);
    void _handleInputArkanoid(bool short_pressed, bool long_pressed);

    // --- Drawing Functions ---
    void _drawScreen(); // Main drawing dispatch function
    // Header & Footer Helpers
    void _drawHeader(const char* title);
    void _drawFooter(const char* hint);
    // Specific Screen Drawers
    void _drawMainMenu();
    void _drawSetAddress();
    void _drawSetStaticLook();
    void _drawChannelBar(int x, int y, int w, int h, uint8_t value, const char* label, bool selected);
    void _drawToggleFlashBootSelect();
    void _drawToggleFlashBootConfirm(bool isConfirmingOn);
    void _drawArkanoid();
    void _drawPoweringOff();

    // --- Helper Functions ---
    void _loadSettings();
    void _saveDmxAddress();
    void _saveStaticLookChannel(int channelIndex);
    void _saveFlashBootSetting();
    uint8_t _calculatePercentage(uint8_t value255); // Calculates 0-100% from 0-255
    int _calculateAcceleratedDiff(int raw_diff); // << ADD Declaration
};

#endif // MENU_SYSTEM_H 