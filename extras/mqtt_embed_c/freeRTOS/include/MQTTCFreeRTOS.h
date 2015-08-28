#ifndef __MQTT_FREERTOS_
#define __MQTT_FREERTOS_

#include "FreeRTOS.h"
#include "task.h"

#include "lwip/api.h"

typedef struct Timer Timer;

struct Timer {
    portTickType end_time;
};

typedef struct Network Network;

struct Network
{
	int my_socket;
	int (*mqttread) (Network*, unsigned char*, int, int);
	int (*mqttwrite) (Network*, unsigned char*, int, int);
	void (*disconnect) (Network*);
};

char expired(Timer*);
void countdown_ms(Timer*, unsigned int);
void countdown(Timer*, unsigned int);
int left_ms(Timer*);

void InitTimer(Timer*);

int freertos_read(Network*, unsigned char*, int, int);
int freertos_write(Network*, unsigned char*, int, int);
void freertos_disconnect(Network*);
void NewNetwork(Network*);

int ConnectNetwork(Network*, char*, int);

#endif
