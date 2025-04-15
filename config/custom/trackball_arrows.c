#include <zephyr/kernel.h>
#include <device.h>
#include <drivers/input/input.h>
#include <zmk/hid.h>
#include <dt-bindings/zmk/keys.h>
#include <logging/log.h>

LOG_MODULE_DECLARE(zmk, CONFIG_ZMK_LOG_LEVEL);  // використовуємо логер ZMK, якщо потрібно

// Індекси для напрямків (для зручності)
#define DIR_LEFT   0
#define DIR_RIGHT  1
#define DIR_UP     2
#define DIR_DOWN   3

// Стан утримання стрілок
static bool arrow_pressed[4] = { false, false, false, false };

// Завчасно оголошуємо обробники таймерів для відпускання клавіш
static void release_left(struct k_work *work);
static void release_right(struct k_work *work);
static void release_up(struct k_work *work);
static void release_down(struct k_work *work);

// Відкладені роботи (таймери) для кожної стрілки
static K_WORK_DELAYABLE_DEFINE(left_release_work, release_left);
static K_WORK_DELAYABLE_DEFINE(right_release_work, release_right);
static K_WORK_DELAYABLE_DEFINE(up_release_work, release_up);
static K_WORK_DELAYABLE_DEFINE(down_release_work, release_down);

// Допоміжна функція для відпускання клавіші стрілки
static void release_arrow_key(int dir) {
    switch(dir) {
    case DIR_LEFT:
        if (arrow_pressed[DIR_LEFT]) {
            zmk_hid_keyboard_release(LEFT);   // відпускаємо HID-код клавіші "Left Arrow"
            arrow_pressed[DIR_LEFT] = false;
        }
        break;
    case DIR_RIGHT:
        if (arrow_pressed[DIR_RIGHT]) {
            zmk_hid_keyboard_release(RIGHT);
            arrow_pressed[DIR_RIGHT] = false;
        }
        break;
    case DIR_UP:
        if (arrow_pressed[DIR_UP]) {
            zmk_hid_keyboard_release(UP);
            arrow_pressed[DIR_UP] = false;
        }
        break;
    case DIR_DOWN:
        if (arrow_pressed[DIR_DOWN]) {
            zmk_hid_keyboard_release(DOWN);
            arrow_pressed[DIR_DOWN] = false;
        }
        break;
    }
}

// Таймерні колбеки – викликаються, коли минув час без руху, щоб "відпустити" клавішу
static void release_left(struct k_work *work)   { release_arrow_key(DIR_LEFT); }
static void release_right(struct k_work *work)  { release_arrow_key(DIR_RIGHT); }
static void release_up(struct k_work *work)     { release_arrow_key(DIR_UP); }
static void release_down(struct k_work *work)   { release_arrow_key(DIR_DOWN); }

// Основна функція обробки події руху
static int trackball_arrow_handle_event(const struct device *dev, struct input_event *event,
                                        uint32_t param1, uint32_t param2,
                                        struct zmk_input_processor_state *state) {
    // Очікуємо події відносного руху по осях X або Y
    if (event->type != INPUT_EV_REL) {
        return 0; // не наша подія, нічого не робимо
    }

    // Визначаємо, по якій осі рух
    if (event->code == INPUT_REL_X) {
        int dx = event->value;
        if (dx > 0) {
            // Рух праворуч
            // Якщо зараз затиснута стрілка LEFT і ми змінили напрямок на правий – відпускаємо LEFT
            if (arrow_pressed[DIR_LEFT]) {
                k_work_cancel_delayable(&left_release_work);
                release_arrow_key(DIR_LEFT);
            }
            // Натискаємо RIGHT, якщо ще не натиснута
            if (!arrow_pressed[DIR_RIGHT]) {
                zmk_hid_keyboard_press(RIGHT);
                arrow_pressed[DIR_RIGHT] = true;
            }
            // Переплануємо (або запустимо) таймер відпускання Right на 50 мс 
            k_work_reschedule(&right_release_work, K_MSEC(50));
        } else if (dx < 0) {
            // Рух ліворуч
            if (arrow_pressed[DIR_RIGHT]) {
                k_work_cancel_delayable(&right_release_work);
                release_arrow_key(DIR_RIGHT);
            }
            if (!arrow_pressed[DIR_LEFT]) {
                zmk_hid_keyboard_press(LEFT);
                arrow_pressed[DIR_LEFT] = true;
            }
            k_work_reschedule(&left_release_work, K_MSEC(50));
        }
        // Якщо dx == 0, нічого не робимо (немає руху по X)
    }
    else if (event->code == INPUT_REL_Y) {
        int dy = event->value;
        if (dy > 0) {
            // Рух вниз
            if (arrow_pressed[DIR_UP]) {
                k_work_cancel_delayable(&up_release_work);
                release_arrow_key(DIR_UP);
            }
            if (!arrow_pressed[DIR_DOWN]) {
                zmk_hid_keyboard_press(DOWN);
                arrow_pressed[DIR_DOWN] = true;
            }
            k_work_reschedule(&down_release_work, K_MSEC(50));
        } else if (dy < 0) {
            // Рух вгору
            if (arrow_pressed[DIR_DOWN]) {
                k_work_cancel_delayable(&down_release_work);
                release_arrow_key(DIR_DOWN);
            }
            if (!arrow_pressed[DIR_UP]) {
                zmk_hid_keyboard_press(UP);
                arrow_pressed[DIR_UP] = true;
            }
            k_work_reschedule(&up_release_work, K_MSEC(50));
        }
        // dy == 0 -> немає руху по Y
    }

    return 0;  // повідомляємо, що подія оброблена (подальші процесори не потрібні)
}

// Ініціалізація драйвера (можна залишити мінімальною)
static int trackball_arrow_init(const struct device *dev) {
    // Додаткова ініціалізація, якщо потрібна (в даному випадку все готово статично)
    return 0;
}

// Оголошення драйвера для Zephyr
static const struct zmk_input_processor_driver_api arrow_processor_api = {
    .handle_event = trackball_arrow_handle_event
};

// Зв'язуємо драйвер з нашим Devicetree-вузлом
DEVICE_DT_DEFINE(DT_NODELABEL(trackball_arrow_processor),
                 trackball_arrow_init, NULL,
                 NULL, NULL,
                 APPLICATION, CONFIG_APPLICATION_INIT_PRIORITY,
                 &arrow_processor_api);
