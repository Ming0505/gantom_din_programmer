#ifndef GANTOM_BOOT_SPLASH_H
#define GANTOM_BOOT_SPLASH_H

// src/view/gantom_boot_splash.h

#include "../factory_test/factory_test.h" // Include FactoryTest dependency
#include <stdint.h>

// Class definition for the Gantom Boot Splash screen
class GantomBootSplash {
public:
    // Constructor: Takes a pointer to the FactoryTest object
    GantomBootSplash(FactoryTest* ft);

    // Function to display the splash screen for a given duration
    void show(uint32_t timeout_ms);

private:
    // Pointer to the FactoryTest instance for hardware interaction
    FactoryTest* _ft;

    // Private helper function to draw the actual splash screen content
    void _drawSplash();
};

#endif // GANTOM_BOOT_SPLASH_H 