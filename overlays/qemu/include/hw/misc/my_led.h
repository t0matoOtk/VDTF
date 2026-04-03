#ifndef HW_MISC_MY_LED_H
#define HW_MISC_MY_LED_H
#include "qom/object.h"
#include "hw/core/sysbus.h"
#define TYPE_MY_LED "my-led"
OBJECT_DECLARE_SIMPLE_TYPE(MyLEDState, MY_LED)
struct MyLEDState {
    SysBusDevice parent_obj;
    MemoryRegion iomem;
    uint32_t led_status;
};
#endif