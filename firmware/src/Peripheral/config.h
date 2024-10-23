#ifndef __config_h__
#define __config_h__
#include <Arduino.h>
#include <driver/i2s.h>
#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <ArduinoJson.h>
#include <ESPmDNS.h>  // Thư viện mDNS để ánh xạ tên miền cục bộ
#include <nvs_flash.h>
#include <nvs.h>
#include <ArduinoWebsockets.h>
#include "I2SMicSampler.h"
#include "I2SSampler.h"


// are you using an I2S microphone - comment this out if you want to use an analog mic and ADC input
#define USE_I2S_MIC_INPUT

// I2S Microphone Settings
// Which channel is the I2S microphone on? I2S_CHANNEL_FMT_ONLY_LEFT or I2S_CHANNEL_FMT_ONLY_RIGHT
// #define I2S_MIC_CHANNEL I2S_CHANNEL_FMT_ONLY_LEFT
#define I2S_MIC_CHANNEL I2S_CHANNEL_FMT_ONLY_RIGHT
#define I2S_MIC_SERIAL_CLOCK GPIO_NUM_33  //SCK
#define I2S_MIC_LEFT_RIGHT_CLOCK GPIO_NUM_26  //WS
#define I2S_MIC_SERIAL_DATA GPIO_NUM_25 //SD

// Analog Microphone Settings - ADC1_CHANNEL_7 is GPIO35
#define ADC_MIC_CHANNEL ADC1_CHANNEL_7

#define I2S_PORT I2S_NUM_0
#define I2S_SAMPLE_RATE   (16000)
#define I2S_SAMPLE_BITS   (16)
#define I2S_READ_LEN      (4096)


#define ESP32_WIFI_CONFIG              1
#define ESP32_VOICE_WAKEUP             1
#define ESP32_VOICE_CONVERSATION       1


#define LED_PIN             GPIO_NUM_2
#define BUTTON              GPIO_NUM_32

#define TIME_OUT             1000 //10s


typedef enum {
    WAITING_WIFI_CONNECT,
    WIFI_DISCONNECT,
    WAIT_INPUT,
    ROBOT_ONLINE,  //3
    ROBOT_SEND_VOICE,
    ROBOT_RECEIVED_RESPONE,
    ROBOT_SEND_ESP2,
    SENSOR_ON,
    SENSOR_OFF,
    TIMEOUT,
    SLEEPING,
    ROBOT_WAKEUP,  //11
    BUTTON_DOWN,
    SOCKET_CONNECT,
    SOCKET_DISCONNECT,
    VOICE_RESPONSE,
    SEND_VOICE
} status_robot;


using namespace websockets;
extern int status_Robot; //WAITING_WIFI_CONNECT


void LED_ON(void);
void LED_OFF(void);



#endif


