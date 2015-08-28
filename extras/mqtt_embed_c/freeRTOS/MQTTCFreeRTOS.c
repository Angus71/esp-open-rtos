
#include "MQTTCFreeRTOS.h"

#include "lwip/err.h"
#include "lwip/sockets.h"
#include "lwip/sys.h"
#include "lwip/netdb.h"
#include "lwip/dns.h"
#include "lwip/api.h"

char expired(Timer* timer) {
    return (timer->end_time <= xTaskGetTickCount());
}

void countdown_ms(Timer* timer, unsigned int timeout) {
    timer->end_time = xTaskGetTickCount() + (timeout / portTICK_RATE_MS);
}

void countdown(Timer* timer, unsigned int timeout) {
    timer->end_time = xTaskGetTickCount() + (timeout * configTICK_RATE_HZ);
}

int left_ms(Timer* timer) {
    portTickType cur = xTaskGetTickCount();
    return (timer->end_time > cur)?(timer->end_time - cur)*portTICK_RATE_MS:0;
}

void InitTimer(Timer* timer) {
    timer->end_time = 0;
}

int freertos_read(Network* n, unsigned char* buffer, int len, int timeout_ms) {
	int timeout = timeout_ms;
	setsockopt(n->my_socket, SOL_SOCKET, SO_RCVTIMEO, (char *)&timeout, sizeof(int));

	int bytes = 0;
	while (bytes < len)
	{
		int rc = recv(n->my_socket, &buffer[bytes], (size_t)(len - bytes), 0);
		if (rc == -1)
		{
			if (errno != ENOTCONN && errno != ECONNRESET)
			{
				bytes = -1;
				break;
			}
		}
		else
			bytes += rc;
	}
	return bytes;
}

int freertos_write(Network* n, unsigned char* buffer, int len, int timeout_ms) {
	int timeout = timeout_ms;
	setsockopt(n->my_socket, SOL_SOCKET, SO_SNDTIMEO, (char *)&timeout, sizeof(int));

	return write(n->my_socket, buffer, len);
}

void freertos_disconnect(Network* n) {
	close(n->my_socket);
}

void NewNetwork(Network* n) {
    n->my_socket = 0;
    n->mqttread = freertos_read;
    n->mqttwrite = freertos_write;
    n->disconnect = freertos_disconnect;
}

int ConnectNetwork(Network* n, char* addr, int port)
{
	int type = SOCK_STREAM;
	struct sockaddr_in address;
	int rc = -1;
	int32_t family = AF_INET;
	struct addrinfo *result = NULL;
	struct addrinfo hints = {0, AF_UNSPEC, SOCK_STREAM, IPPROTO_TCP, 0, NULL, NULL, NULL};

	if ((rc = getaddrinfo(addr, NULL, &hints, &result)) == 0)
	{
		struct addrinfo* res = result;

		/* prefer ip4 addresses */
		while (res)
		{
			if (res->ai_family == AF_INET)
			{
				result = res;
				break;
			}
			res = res->ai_next;
		}

		if (result->ai_family == AF_INET)
		{
			address.sin_port = htons(port);
			address.sin_family = family = AF_INET;
			address.sin_addr = ((struct sockaddr_in*)(result->ai_addr))->sin_addr;
		}
		else
			rc = -1;

		freeaddrinfo(result);
	}

	if (rc == 0)
	{
		n->my_socket = socket(family, type, 0);
		if (n->my_socket != -1)
		{
			rc = connect(n->my_socket, (struct sockaddr*)&address, sizeof(address));
		}
	}

	return rc;
}
