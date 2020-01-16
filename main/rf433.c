#include "rf433.h"

#include <driver/gpio.h>
#include <esp_timer.h>
#include <esp_log.h>

static gpio_num_t RF433_GPIO = (gpio_num_t) CONFIG_RF433_RECEIVER_GPIO;

xQueueHandle rfcode_event_queue = NULL;

typedef struct {
    int level;
    int time_us;
} Event;

typedef struct {
    int base_pulse_width;
    int min_bit_width;
    int max_bit_width;
    int min_sync_width;
    int max_sync_width;
} ProtocolSpec;


inline unsigned int divint(int a, int b) {
    return ((1024 * a / b) + 512) / 1024;
}

static void IRAM_ATTR gpio_isr_handler(void *arg) {
    static Event prev;
    static ProtocolSpec spec;

    static int high_pulse_us;
    static int low_pulse_us;
    static int bit_num = -1;    // -1 indicates we do not reading code yet (looking for SYNC)
    static uint32_t data;

    Event now = {
        .time_us = esp_timer_get_time(),
        .level = gpio_get_level(RF433_GPIO),  // get level of next pulse because we measure AFTER edge!
    };
    Event pulse = {
        .time_us = now.time_us - prev.time_us,
        .level = prev.level,
    };
    // sanity check
    if (now.level == prev.level) {
        // we missed some interrupts probably because of RF noise
        bit_num = -1;
        prev = now;
        return;
    }
    prev = now;

    // save pulse width
    switch (pulse.level) {
        case 1: // first pulse (HIGH)
            high_pulse_us = pulse.time_us;
            return;                            // <-- exit after high pulse (only half-bit received)
        case 0: // second pulse (LOW)
            low_pulse_us = pulse.time_us;
            break;                             // <-- continue after low pulse
        default:
            return;
    }

    if (bit_num == -1) {                       // <-- looking for SYNC
        // check if the pulse looks like SYNC
        //  +---+                           +
        //  | 1 |            31             |
        //  +   +---------------------------+
        if (high_pulse_us == 0) return;
        int r = divint(low_pulse_us, high_pulse_us);
        if (r < 28 || r > 32) { // not a SYNC
            return;
        }
        // sync pulse found

        int sync_width = high_pulse_us + low_pulse_us;
        // does it look like previous SYNC?
        if (sync_width >= spec.min_sync_width && sync_width <= spec.max_sync_width) {
            // just use already calculated spec
        } else {
            spec.base_pulse_width = sync_width / 32;
            //  +---+         +                   +---------+   +
            //  | 1 |    3    |        or         |    3    | 1 |
            //  +   +---------+                   +         +---+
            int base_bit_width = spec.base_pulse_width * 4;
            int percent_4 = divint(base_bit_width, 25);
            spec.min_bit_width = base_bit_width - percent_4;
            spec.max_bit_width = base_bit_width + percent_4;

            int percent_1 = divint(sync_width, 100);
            spec.min_sync_width = sync_width - percent_1;
            spec.max_sync_width = sync_width + percent_1;
        }
        // start reading data bits
        bit_num = 0;
        data = 0;
        return;
    }

    // check next bit width within base pulse width (+- 4%)
    int bit_width = high_pulse_us + low_pulse_us;
    if (bit_width < spec.min_bit_width) {
        // looks like noise
        bit_num = -1;
        return;
    } else if (bit_width > spec.max_bit_width) {
        // probably next SYNC?
        if (bit_width >= spec.min_sync_width && bit_width <= spec.max_sync_width) {
            if (high_pulse_us == 0) {  // not a SYNC
                bit_num = -1;
                return;
            }
            int r = divint(low_pulse_us, high_pulse_us);
            if (r >= 28 || r <= 32) { // it's a SYNC
                // we are done with that code
                xQueueSendFromISR(rfcode_event_queue, &data, NULL);

                // start reading data bits for next code
                bit_num = 0;
                data = 0;
                return;
            }
        }
        // not a bit and not a SYNC
        bit_num = -1;
        return;
    }

    // write bit
    data <<= 1;
    if (high_pulse_us > low_pulse_us) {  // TODO: check pulse's widths
        data |= 0x1;
    }

    bit_num++;
    if (bit_num == 32) bit_num = -1;  // data overflow

    // we send code only when we get SYNC of next one. in that case we drop last code that doesn't have SYNC after it.
    // that is made intentionally. some devices send broken data in last code of the sequence.
}

void rf433_init(void) {
    rfcode_event_queue = xQueueCreate(512, sizeof(uint32_t));

    // setup GPIO
    gpio_config_t io_conf;
    io_conf.intr_type = GPIO_INTR_ANYEDGE;
    io_conf.pin_bit_mask = (uint32_t) 0x1 << RF433_GPIO;
    io_conf.mode = GPIO_MODE_INPUT;
    io_conf.pull_up_en = GPIO_PULLUP_DISABLE;
    io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
    ESP_ERROR_CHECK(gpio_config(&io_conf));

    // setup GPIO interrupt
    gpio_install_isr_service(0);
    gpio_isr_handler_add(RF433_GPIO, gpio_isr_handler, NULL);
}
