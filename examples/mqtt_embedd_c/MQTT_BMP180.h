/*
 * MQTT_BMP180.h
 *
 *  Created on: 28.08.2015
 *      Author: fbargste
 */

#ifndef MQTT_BMP180_H_
#define MQTT_BMP180_H_

#include "FreeRTOS.h"
#include "queue.h"

#define SCL_PIN GPIO_ID_PIN((0))
#define SDA_PIN GPIO_ID_PIN((2))

// Measure bmp180 every second
#define MQTT_BMP180_MEASURE_TIME (1000/portTICK_RATE_MS)

void MQTT_BMP180_Init(xQueueHandle*);


#endif /* MQTT_BMP180_H_ */
