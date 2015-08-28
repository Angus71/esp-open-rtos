# Component makefile for mqtt

MQTT_DIR = $(ROOT)extras/mqtt_embed_c/
INC_DIRS += $(MQTT_DIR)mqtt/MQTTPacket/src $(MQTT_DIR)freeRTOS/include $(MQTT_DIR)mqtt/MQTTClient-C/src

# args for passing into compile rule generation
extras/mqtt_embed_c_ROOT = $(ROOT)/extras/mqtt_embed_c
extras/mqtt_embed_c_SRC_DIR = $(MQTT_DIR)mqtt/MQTTPacket/src $(MQTT_DIR)mqtt/MQTTClient-C/src
extras/mqtt_embed_c_EXTRA_SRC_FILES = $(MQTT_DIR)freeRTOS/MQTTCFreeRTOS.c

# disable warnings and do not load header files from the directory of the current file (Needed for MQTTClient.h to be found in freeRTOS/include)
extras/mqtt_embed_c_CFLAGS = $(CFLAGS) -Wno-unused-but-set-variable 

$(eval $(call component_compile_rules,extras/mqtt_embed_c))

# Helpful error if git submodule not initialised
$(extra/mqtt_embed_c_SRC_DIR):
	$(error "MQTT Embedded-C git submodule not installed. Please run 'git submodule init' then 'git submodule update'")


