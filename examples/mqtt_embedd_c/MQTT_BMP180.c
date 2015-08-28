/*
 * MQTT_BMP180.c
 *
 *  Created on: 28.08.2015
 *      Author: fbargste
 */
#include "espressif/esp_common.h"
#include "espressif/sdk_private.h"

#include "FreeRTOS.h"
#include "timers.h"
#include "queue.h"

// BMP180 driver
#include "bmp180/bmp180.h"

#include "MQTT.h"
#include "MQTT_BMP180.h"

// Timer used for triggering measurements
xTimerHandle timerHandle;
xQueueHandle* com_queue;

// Own BMP180 User Inform Implementation
static bool MQTT_BMP180_informUser(const xQueueHandle* resultQueue, uint8_t cmd, bmp180_temp_t temperatue, bmp180_press_t pressure)
{
    mqtt_event_t ev;

    ev.event_type = EVT_MQTT_BMP180;
    ev.event_data.bmp180_data.cmd = cmd;
    ev.event_data.bmp180_data.temperatue = temperatue;
    ev.event_data.bmp180_data.pressure = pressure;

    return (xQueueSend(*resultQueue, &ev, 0) == pdTRUE);
}

// Timer call back
static void MQTT_BMP180_timer_cb(xTimerHandle timerHandle) {
	// Trigger measurement
	bmp180_trigger_measurement(com_queue);
}

void MQTT_BMP180_Init(xQueueHandle* queue) {
    // Use our user inform implementation
    bmp180_informUser = MQTT_BMP180_informUser;

    // Init BMP180 Interface
    bmp180_init(SCL_PIN, SDA_PIN);

    // Store user queue for future use
    com_queue = queue;

    // Create Timer (Trigger a measurement every second)
    timerHandle = xTimerCreate((signed char *)"BMP180 Trigger", MQTT_BMP180_MEASURE_TIME, pdTRUE, NULL, MQTT_BMP180_timer_cb);

    if(timerHandle!=NULL) {
    	if (xTimerStart(timerHandle, 0) == pdPASS) {
    		printf("%s: Started MQTT_BMP180 measure trigger. Update Rate: %lu ms\n", __FUNCTION__, MQTT_BMP180_MEASURE_TIME*portTICK_RATE_MS);
    	}
    }
}
