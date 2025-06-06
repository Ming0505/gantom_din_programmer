#pragma once
#include <smooth_ui_toolkit.h>
#include "../factory_test/factory_test.h"

// Forward declarations for the sub-menus
class DMXAddressMenu;
class StaticLookMenu;
class ToggleBootFlashMenu;

using namespace SmoothUIToolKit;
using namespace SmoothUIToolKit::SelectMenu;

class PM_StaticLookMenu : public SmoothOptions {
public:
    
    void onReadInput() override;
    void onRender() override;
    void onPress() override;
    void onClick() override;
    void onOpenEnd() override;
    
    void update(uint32_t ms);
    void set_ftptr(FactoryTest* ft);
    FactoryTest* get_ftptr();
    
}; 
