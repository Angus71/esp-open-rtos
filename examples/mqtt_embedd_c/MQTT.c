/* Simple example for MQTT
 *
 * This sample code is in the public domain.
 */
#include "espressif/esp_common.h"
#include "espressif/sdk_private.h"
#include "ssid_config.h"

// FreeRTOS includes
#include "FreeRTOS.h"
#include "task.h"
#include "timers.h"
#include "queue.h"

// MQTT includes
#include "MQTTClient.h"

// Local application includes
#include "MQTT.h"
#include "MQTT_BMP180.h"

// Communication Queue
static xQueueHandle mainqueue;

// handle MQTT_BMP180 event
static void handle_EVT_MQTT_BMP180(Client* c, bmp180_result_t* measurement) {
	MQTTMessage msg;
	// We want an acknowledge from the brocker
	msg.qos = QOS1;
	msg.retained = 1;
	msg.payload = (void *)&(measurement->pressure);
	msg.payloadlen = sizeof(measurement->pressure);

	int rc = MQTTPublish(c, MQTT_PRESSURE, &msg);
	printf("%s: MTTQPublish(%s):= %d\n", __FUNCTION__, MQTT_PRESSURE, rc);

	msg.payload = (void *)&(measurement->temperatue);
	msg.payloadlen = sizeof(measurement->temperatue);

	rc = MQTTPublish(c, MQTT_TEMPERATURE, &msg);
	printf("%s: MTTQPublish(%s):= %d\n", __FUNCTION__, MQTT_TEMPERATURE, rc);
}


// Check for communiction events
void MQTT_task(void *pvParameters)
{
    // Received pvParameters is communication queue
    xQueueHandle *com_queue = (xQueueHandle *)pvParameters;

    printf("%s: Started user interface task\n", __FUNCTION__);

    // To reduce stack size use dynamic memory
    uint8_t* buf = malloc(MQTT_DEFAULT_BUF_SIZE);
    uint8_t* readBuffer = malloc(MQTT_DEFAULT_BUF_SIZE);
    Network* n = malloc(sizeof(Network));
    Client*  c = malloc(sizeof(Client));

    // Are we connected?
    bool online = false;

    while(1)
    {
		int rc = 0;
        mqtt_event_t ev;

        xQueueReceive(*com_queue, &ev, portMAX_DELAY);

        switch(ev.event_type)
        {
        case EVT_MQTT_WIFI:
        	switch((uint8_t)(ev.event_data.received_data&0xFF)) {
        	case STATION_GOT_IP:
				// Init Network
				NewNetwork(n);

				rc = ConnectNetwork(n, MQTT_BROKER, MQTT_PORT);
				if (rc==0) {
					printf("%s: Connected to network %s:%d\n", __FUNCTION__, MQTT_BROKER, MQTT_PORT);
					// Init client
					MQTTClient(c, n, MQTT_DEFAULT_TIMEOUT, buf, MQTT_DEFAULT_BUF_SIZE, readBuffer, MQTT_DEFAULT_BUF_SIZE);

					// Create Data Connection
					MQTTPacket_connectData data = MQTTPacket_connectData_initializer;
					data.willFlag = 0;
					data.MQTTVersion = 3;
					data.clientID.cstring = "MQTT_task";
					data.username.cstring = NULL;
					data.password.cstring = NULL;
					data.keepAliveInterval = 10;
					data.cleansession = 1;

					rc = MQTTConnect(c, &data);
					printf("%s: Connected %d\n", __FUNCTION__, rc);

					online = true;
				} else {
					printf("%s: Failed to connect to Broker %s:%d\n", __FUNCTION__, MQTT_BROKER, MQTT_PORT);
				}
				break;
        	default:
        		// All other events mean no connection anymore ...
        		if (online) {
					printf("%s: Disconnect\n", __FUNCTION__);
					rc = MQTTDisconnect(c);
					printf("%s: Disconnect result %d\n", __FUNCTION__, rc);

					n->disconnect(n);

					online = false;
        		}
        		break;
        	}
            break;
        case EVT_MQTT_BMP180:
        	if (online)
        		handle_EVT_MQTT_BMP180(c, &ev.event_data.bmp180_data);
        	else
        		printf("%s: Received BMP180 event, but not online ...", __FUNCTION__);
        	break;
        default:
            break;
        }
    }
}

// Check for communiction events
void WIFI_task(void *pvParameters) {
    // Received pvParameters is communication queue
    xQueueHandle *com_queue = (xQueueHandle *)pvParameters;

    printf("%s: Started WIFI task\n", __FUNCTION__);

    uint8_t old_wifi_state = STATION_IDLE;

    while(1)
    {
        mqtt_event_t ev;

        // Get current wifi state
        uint8_t current_wifi_state = sdk_wifi_station_get_connect_status();

        // We found a change
        if (current_wifi_state != old_wifi_state) {
        	printf("%s: Wifi STATE changed from %d -> %d\n", __FUNCTION__, old_wifi_state, current_wifi_state);
    		// Create Event
        	ev.event_type = EVT_MQTT_WIFI;
    		ev.event_data.received_data = (uint32_t)current_wifi_state;

    		xQueueSend(*com_queue, &ev, 0);

    		old_wifi_state = current_wifi_state;
        }
        // Wait some ticks before checking again (1 second)
        vTaskDelay(1000/portTICK_RATE_MS);
    }
}


// Setup HW
void user_setup(void)
{
    // Set UART Parameter
    sdk_uart_div_modify(0, UART_CLK_FREQ / 115200);

    // Give the UART some time to settle
    sdk_os_delay_us(500);

    struct sdk_station_config config = {
        .ssid = WIFI_SSID,
        .password = WIFI_PASS,
    };

    /* required to call wifi_set_opmode before station_set_config */
    sdk_wifi_set_opmode(STATION_MODE);
    sdk_wifi_station_set_config(&config);
}

void user_init(void)
{
    // Setup HW
    user_setup();

    // Just some infomations
    printf("\n");
    printf("SDK version : %s\n", sdk_system_get_sdk_version());
    printf("GIT version : %s\n", GITSHORTREV);

    // Create Main Communication Queue
    mainqueue = xQueueCreate(10, sizeof(mqtt_event_t));

    // Create mqtt interface task
    xTaskCreate(MQTT_task, (signed char *)"MQTT_task", 2048, &mainqueue, 2, NULL);
    // Create WIFI interface task
    xTaskCreate(WIFI_task, (signed char *)"WIFI_task", 256, &mainqueue, 1, NULL);

    // Init Measurement of BMP180
    MQTT_BMP180_Init(&mainqueue);
}
