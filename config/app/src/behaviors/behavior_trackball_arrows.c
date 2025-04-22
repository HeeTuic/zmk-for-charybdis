#include <zephyr/device.h>
#include <zephyr/init.h>
#include <zephyr/logging/log.h>
#include <zmk/behavior.h>
#include <zmk/behavior_driver.h>
#include <zmk/events/relative_mouse_event.h>
#include <zmk/events/keycode_state_changed.h>
#include <zmk/keycode.h>

LOG_MODULE_DECLARE(zmk, CONFIG_ZMK_LOG_LEVEL);

#define MOVE_THRESHOLD 10

static int arrows_sensor_binding_triggered(struct zmk_behavior_binding *binding,
                                           struct zmk_behavior_binding_event event) {
    // У цьому прикладі нічого не виконується при натисканні, бо логіка йде через relative event
    return 0;
}

static int arrows_sensor_binding_released(struct zmk_behavior_binding *binding,
                                          struct zmk_behavior_binding_event event) {
    return 0;
}

static void arrows_handle_rel_report(struct zmk_relative_mouse_event *ev) {
    int16_t x = ev->rel_x;
    int16_t y = ev->rel_y;

    if (x > MOVE_THRESHOLD) {
        raise_zmk_keycode_state_changed(ZMK_HID_USAGE_KEY_RIGHT_ARROW, true);
        raise_zmk_keycode_state_changed(ZMK_HID_USAGE_KEY_RIGHT_ARROW, false);
    } else if (x < -MOVE_THRESHOLD) {
        raise_zmk_keycode_state_changed(ZMK_HID_USAGE_KEY_LEFT_ARROW, true);
        raise_zmk_keycode_state_changed(ZMK_HID_USAGE_KEY_LEFT_ARROW, false);
    }

    if (y > MOVE_THRESHOLD) {
        raise_zmk_keycode_state_changed(ZMK_HID_USAGE_KEY_DOWN_ARROW, true);
        raise_zmk_keycode_state_changed(ZMK_HID_USAGE_KEY_DOWN_ARROW, false);
    } else if (y < -MOVE_THRESHOLD) {
        raise_zmk_keycode_state_changed(ZMK_HID_USAGE_KEY_UP_ARROW, true);
        raise_zmk_keycode_state_changed(ZMK_HID_USAGE_KEY_UP_ARROW, false);
    }
}

static int arrows_behavior_listener(const struct zmk_event_header *eh) {
    if (is_zmk_relative_mouse_event(eh)) {
        arrows_handle_rel_report(cast_zmk_relative_mouse_event(eh));
        return ZMK_EV_EVENT_BUBBLE;
    }
    return ZMK_EV_EVENT_CONTINUE;
}

static const struct zmk_behavior_driver_api behavior_driver_api = {
    .binding_pressed = arrows_sensor_binding_triggered,
    .binding_released = arrows_sensor_binding_released,
};

ZMK_LISTENER(trackball_arrows, arrows_behavior_listener);
ZMK_SUBSCRIPTION(trackball_arrows, zmk_relative_mouse_event);

BEHAVIOR_DT_INST_DEFINE(0, behavior_driver_api);
