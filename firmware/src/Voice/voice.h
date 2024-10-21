#ifndef __voice_h__
#define __voice_h__



#include <Arduino.h>
#include <driver/i2s.h>
#include <esp_task_wdt.h>
#include "I2SMicSampler.h"
#include "ADCSampler.h"
#include <WiFi.h>
#include <ArduinoWebsockets.h>
#include "driver/i2s.h"
#include <HardwareSerial.h>
#include "../Peripheral/config.h"
#include "../CommandDetector.h"
#include "../CommandProcessor.h"
#include "I2SMicSampler.h"
#include "I2SSampler.h"
#include "Cert/cert.h"



extern i2s_config_t i2sMicConfig;
extern i2s_pin_config_t i2s_mic_pins;
extern char* i2s_read_buff;
extern TaskHandle_t voiceWakeupTaskHandle;
extern TaskHandle_t voiceConversationTaskHandle;
extern I2SSampler *i2s_sampler;


extern const char* websockets_server_host;
extern const uint16_t websockets_server_port;

using namespace websockets;
extern WebsocketsClient client;

extern uint8_t* flash_write_buff;

extern int currentState;   // Trạng thái 1 là thu và gửi dữ liệu, 2 là gửi link qua UART và chờ phản hồi
extern bool linkSent;  // Biến cờ để theo dõi việc gửi link chỉ một lần

extern String linkAudioToSpeech;  // Lưu link âm thanh để gửi qua UART


extern SemaphoreHandle_t xSemaphore;


void i2s_adc_task();
void i2s_adc_convert(uint8_t *i2sData);
void sendLinkToESP2();
void onMessageCallback(WebsocketsMessage message);
void i2s_adc_data_scale(uint8_t* d_buff, uint8_t* s_buff, uint32_t len);
void onEventsCallback(WebsocketsEvent event, String data);
void startI2S();
void stopI2S();
void voiceWakeupTask(void *param);
void voiceConversationTask(void *param);



#endif


