#pragma once

#include <freertos/FreeRTOS.h>
#include "freertos/queue.h"

extern xQueueHandle rfcode_event_queue;

void rf433_init(void);
