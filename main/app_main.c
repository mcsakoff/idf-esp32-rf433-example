#include "rf433.h"

#include <nvs_flash.h>
#include <freertos/task.h>
#include <esp_log.h>

static const char TAG[] = "rf433";


void rf433Task(void *args) {
    ESP_LOGI(TAG, "task started");

    RFRawEvent rfcode;
    for (;;) {
        if (xQueueReceive(rfcode_event_queue, &rfcode, portMAX_DELAY)) {
            switch (rfcode.action) {
                case RFCODE_START:
                    ESP_LOGI(TAG, "start:    %06X (protocol: %02X, %i bits)", rfcode.raw_code, rfcode.protocol, rfcode.bits);
                    break;
                case RFCODE_CONTINUE:
                    ESP_LOGI(TAG, "continue: %06X (protocol: %02X, %i bits)", rfcode.raw_code, rfcode.protocol, rfcode.bits);
                    break;
                case RFCODE_STOP:
                    ESP_LOGI(TAG, "stop:     %06X (protocol: %02X, %i bits)", rfcode.raw_code, rfcode.protocol, rfcode.bits);
                    break;
            }
        }
    }

    ESP_LOGI(TAG, "task stopped");
    vTaskDelete(NULL);
}


void app_main(void) {
    // initialize NVS
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    rf433_init();
    xTaskCreate(rf433Task, "rf433Task", 2048, NULL, 6, NULL);
}
