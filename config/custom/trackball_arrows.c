// config/custom/trackball_arrow_processor.c

#include <zephyr/kernel.h>
#include <device.h>
#include <drivers/input/input.h>
#include <zmk/hid.h>
#include <dt-bindings/zmk/keys.h>

#define HOLD_MS 50

static bool pressed[4] = {false};

static void release(int i) {
    if (pressed[i]) {
        zmk_hid_keyboard_release((uint8_t[]){HID_USAGE_KEY_KEYBOARD_LEFT_ARROW, HID_USAGE_KEY_KEYBOARD_RIGHT_ARROW, HID_USAGE_KEY_KEYBOARD_UP_ARROW, HID_USAGE_KEY_KEYBOARD_DOWN_ARROW}[i]);
        pressed[i] = false;
    }
}

static void hold(int i) {
    if (!pressed[i]) {
        zmk_hid_keyboard_press((uint8_t[]){HID_USAGE_KEY_KEYBOARD_LEFT_ARROW, HID_USAGE_KEY_KEYBOARD_RIGHT_ARROW, HID_USAGE_KEY_KEYBOARD_UP_ARROW, HID_USAGE_KEY_KEYBOARD_DOWN_ARROW}[i]);
        pressed[i] = true;
    }
}

static void delayed_release(struct k_work *work);
K_WORK_DELAYABLE_DEFINE(release_work[4], delayed_release);

static void delayed_release(struct k_work *work) {
    for (int i = 0; i < 4; i++) {
        if (work == &release_work[i].work) {
            release(i);
        }
    }
}

static int processor(const struct device *dev, struct input_event *evt, uint32_t pm1, uint32_t pm2, struct zmk_input_processor_state *state) {
    if (evt->type != INPUT_EV_REL) return 0;

    if (evt->code == INPUT_REL_X) {
        if (evt->value > 0) {
            release(0); hold(1); k_work_reschedule(&release_work[1], K_MSEC(HOLD_MS));
        } else if (evt->value < 0) {
            release(1); hold(0); k_work_reschedule(&release_work[0], K_MSEC(HOLD_MS));
        }
    }

    if (evt->code == INPUT_REL_Y) {
        if (evt->value > 0) {
            release(2); hold(3); k_work_reschedule(&release_work[3], K_MSEC(HOLD_MS));
        } else if (evt->value < 0) {
            release(3); hold(2); k_work_reschedule(&release_work[2], K_MSEC(HOLD_MS));
        }
    }

    return 0;
}

static int init(const struct device *dev) { return 0; }

static const struct zmk_input_processor_driver_api api = {.handle_event = processor};

DEVICE_DT_DEFINE(DT_NODELABEL(trackball_arrow_processor), init, NULL, NULL, NULL, APPLICATION, CONFIG_APPLICATION_INIT_PRIORITY, &api);
