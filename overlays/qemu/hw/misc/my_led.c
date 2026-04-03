#include "qemu/osdep.h"
#include "hw/misc/my_led.h"
#include "qemu/module.h"

static uint64_t led_read(void *opaque, hwaddr offset, unsigned size) {
    MyLEDState *s = MY_LED(opaque);
    printf("\n [QEMU LED] READ called!\n"); fflush(stdout);
    return s->led_status;
}

static void led_write(void *opaque, hwaddr offset, uint64_t value, unsigned size) {
    MyLEDState *s = MY_LED(opaque);
    s->led_status = value & 1;
    if (s->led_status) {
        printf("\n [QEMU LED] 💡 LED 亮了！\n"); fflush(stdout);
    } else {
        printf("\n [QEMU LED] 🌑 LED 滅了！\n"); fflush(stdout);
    }
}

static const MemoryRegionOps led_ops = {
    .read = led_read,
    .write = led_write,
    .endianness = DEVICE_NATIVE_ENDIAN,
};

static void my_led_init(Object *obj) {
    MyLEDState *s = MY_LED(obj);
    memory_region_init_io(&s->iomem, obj, &led_ops, s, "my-led-regs", 0x100);
    sysbus_init_mmio(SYS_BUS_DEVICE(obj), &s->iomem);
}

static const TypeInfo my_led_info = {
    .name          = TYPE_MY_LED,
    .parent        = TYPE_SYS_BUS_DEVICE,
    .instance_size = sizeof(MyLEDState),
    .instance_init = my_led_init,
};

static void my_led_register_types(void) {
    type_register_static(&my_led_info);
}
type_init(my_led_register_types)

