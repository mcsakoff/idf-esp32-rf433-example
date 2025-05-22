#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <esp_log.h>

#include <driver/rf_receiver.h>

static const char *TAG = "main";

void rf_task(void *args) {
    ESP_LOGI(TAG, "task started");

    // configure the driver
    rf_config_t config = RF_DEFAULT_CONFIG(CONFIG_RF_RECEIVER_GPIO);
    ESP_ERROR_CHECK(rf_config(&config));

    // install the driver
    ESP_ERROR_CHECK(rf_driver_install(0));

    // get the actions queue
    QueueHandle_t events = NULL;
    ESP_ERROR_CHECK(rf_get_events_handle(&events));

    rf_event_t rfcode;
    for (;;) {
        if (xQueueReceive(events, &rfcode, portMAX_DELAY)) {
            switch (rfcode.action) {
                case RF_ACTION_START:
                    ESP_LOGI(TAG, "start:    %06llX (protocol: %02hX, %i bits)", rfcode.raw_code, rfcode.protocol, rfcode.bits);
                    break;
                case RF_ACTION_CONTINUE:
                    ESP_LOGI(TAG, "continue: %06llX (protocol: %02hX, %i bits)", rfcode.raw_code, rfcode.protocol, rfcode.bits);
                    break;
                case RF_ACTION_STOP:
                    ESP_LOGI(TAG, "stop:     %06llX (protocol: %02hX, %i bits)", rfcode.raw_code, rfcode.protocol, rfcode.bits);
                    break;
            }
        }
    }

    ESP_LOGI(TAG, "task stopped");
    vTaskDelete(NULL);
}

void app_main(void) {
    xTaskCreate(rf_task, "rf_task", 2048, NULL, 10, NULL);
}
