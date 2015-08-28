/*
 * MQTT.h
 *
 *  Created on: 28.08.2015
 *      Author: fbargste
 */

#ifndef MQTT_H_
#define MQTT_H_

#define MQTT_BROKER           "192.168.3.154"
#define MQTT_PORT             1883
#define MQTT_DEFAULT_TIMEOUT  1000
#define MQTT_DEFAULT_BUF_SIZE 100

#define MQTT_PRESSURE         "Home/Office/Pressure"
#define MQTT_TEMPERATURE      "Home/Office/Temperature"

#define EVT_MQTT_WIFI    0x01
#define EVT_MQTT_CONNECT 0x02
#define EVT_MQTT_BMP180  0x03

// BMP180 driver .. Needed for bmp180 data types
#include "bmp180/bmp180.h"

typedef struct
{
    uint8_t  event_type;
    union {
    	uint32_t received_data;
    	bmp180_result_t bmp180_data;
    } event_data;
} mqtt_event_t;

#endif /* MQTT_H_ */
