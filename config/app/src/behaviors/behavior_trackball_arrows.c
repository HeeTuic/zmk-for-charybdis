// behavior_trackball_arrows.c

#define DT_DRV_COMPAT zmk_behavior_trackball_arrows

#include <zephyr/device.h>
#include <drivers/behavior.h>
#include <zephyr/logging/log.h>
#include <zmk/hid.h>
#include <zmk/behavior.h>
#include <dt-bindings/zmk/hid_usage.h>

LOG_MODULE_DECLARE(zmk, CONFIG_ZMK_LOG_LEVEL);

static int on_sensor_binding_triggered(const struct device *dev, 
                                       const struct sensor_value *dx,
                                       const struct sensor_value *dy) {

    if (dx->val1 > 0) {
        zmk_hid_press(ZMK_HID_USAGE(HID_USAGE_KEY, HID_USAGE_KEY_KEYBOARD_RIGHT_ARROW));
        zmk_hid_release(ZMK_HID_USAGE(HID_USAGE_KEY, HID_USAGE_KEY_KEYBOARD_RIGHT_ARROW));
    } else if (dx->val1 < 0) {
        zmk_hid_press(ZMK_HID_USAGE(HID_USAGE_KEY, HID_USAGE_KEY_KEYBOARD_LEFT_ARROW));
        zmk_hid_release(ZMK_HID_USAGE(HID_USAGE_KEY, HID_USAGE_KEY_KEYBOARD_LEFT_ARROW));
    }

    if (dy->val1 > 0) {
        zmk_hid_press(ZMK_HID_USAGE(HID_USAGE_KEY, HID_USAGE_KEY_KEYBOARD_DOWN_ARROW));
        zmk_hid_release(ZMK_HID_USAGE(HID_USAGE_KEY, HID_USAGE_KEY_KEYBOARD_DOWN_ARROW));
    } else if (dy->val1 < 0) {
        zmk_hid_press(ZMK_HID_USAGE(HID_USAGE_KEY, HID_USAGE_KEY_KEYBOARD_UP_ARROW));
        zmk_hid_release(ZMK_HID_USAGE(HID_USAGE_KEY, HID_USAGE_KEY_KEYBOARD_UP_ARROW));
    }

    return 0;
}

static const struct behavior_driver_api trackball_arrows_driver_api = {
    .sensor_binding_triggered = on_sensor_binding_triggered,
};

BEHAVIOR_DT_INST_DEFINE(0, NULL, NULL, NULL, NULL,
                        POST_KERNEL, CONFIG_KERNEL_INIT_PRIORITY_DEFAULT,
                        &trackball_arrows_driver_api);
