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
extern TaskHandle_t voiceProcessTaskHandle;
extern I2SSampler *i2s_sampler;


extern const char* websockets_server_host;
extern const uint16_t websockets_server_port;

using namespace websockets;
extern WebsocketsClient client;

extern uint8_t* flash_write_buff;
extern SemaphoreHandle_t xSemaphore;


typedef enum {
    NO_READ,
    GET_VOICE_CONVERSATION,
    SEND_VOICE_SERVER
} state_voice;

class Voice_Conversation
{
public:
    int state = NO_READ;
    bool linkSent = false;
    String linkAudio;

// protected:
//     void addSample(int16_t sample);
//     virtual void configureI2S() = 0;
//     virtual void processI2SData(uint8_t *i2sData, size_t bytesRead) = 0;
//     virtual void processI2SData_scale(uint8_t* s_buff, uint8_t* d_buff, uint32_t len) = 0;
//     i2s_port_t getI2SPort()
//     {
//         return m_i2s_port;
//     }

public:
    void sendLink2ESP(void);
    void sendVoice2Server(const char *buff);
};

extern Voice_Conversation voice_con;


void i2s_adc_task();
void i2s_adc_convert(uint8_t *i2sData);
void sendLinkToESP2();
void onMessageCallback(WebsocketsMessage message);
void i2s_adc_data_scale(uint8_t* d_buff, uint8_t* s_buff, uint32_t len);
void onEventsCallback(WebsocketsEvent event, String data);
void startI2S();
void stopI2S();
void voiceProcessTask(void *param);



#endif


