#include "voice.h"

TaskHandle_t voiceProcessTaskHandle;
I2SSampler *i2s_sampler = new I2SMicSampler(i2s_mic_pins, false);

// Trạng thái 1 là thu và gửi dữ liệu, 2 là gửi link qua UART và chờ phản hồi
int state_Conversation = 0;

// i2s microphone pins
i2s_pin_config_t i2s_mic_pins = {
    .bck_io_num = I2S_MIC_SERIAL_CLOCK,
    .ws_io_num = I2S_MIC_LEFT_RIGHT_CLOCK,
    .data_out_num = I2S_PIN_NO_CHANGE,
    .data_in_num = I2S_MIC_SERIAL_DATA};

i2s_config_t i2sMicConfig = {
    .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_RX),
    .sample_rate = 16000,
    .bits_per_sample = I2S_BITS_PER_SAMPLE_32BIT,
    .channel_format = I2S_MIC_CHANNEL,
    .communication_format = i2s_comm_format_t(I2S_COMM_FORMAT_I2S),
    .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,
    .dma_buf_count = 4,
    .dma_buf_len = 64,
    .use_apll = false,
    .tx_desc_auto_clear = false,
    .fixed_mclk = 0
};

char* i2s_read_buff;

SemaphoreHandle_t xSemaphore;
Voice_Conversation voice_con;



void i2s_adc_convert(uint8_t *i2sData) {
  i2s_adc_data_scale(flash_write_buff, (uint8_t*)i2sData, I2S_READ_LEN);
  client.sendBinary((const char*)flash_write_buff, I2S_READ_LEN);
  client.poll();
}


void onMessageCallback(WebsocketsMessage message) {
  String respond_message = message.data();
  Serial.println(respond_message);

  if (respond_message.indexOf("\"audio\":") != -1) {
    int indexOfText = respond_message.indexOf("\"text\":");
    int indexOfAudio = respond_message.indexOf("\"audio\":");
    int indexOfEmotion = respond_message.indexOf("\"emotion\":");
    String textToSpeech = respond_message.substring(indexOfText + 8, indexOfAudio - 2);
    voice_con.linkAudio = respond_message.substring(indexOfAudio + 9, indexOfEmotion - 2);
    Serial.println("Text: " + textToSpeech);
    Serial.println("Audio Link: " + voice_con.linkAudio);

    voice_con.state = SEND_VOICE_SERVER;
    voice_con.linkSent = false;
  }
}



void i2s_adc_data_scale(uint8_t* d_buff, uint8_t* s_buff, uint32_t len) {
  uint32_t j = 0;
  uint32_t dac_value = 0;
  for (int i = 0; i < len; i += 4) {  // 4 bytes per 32-bit sample
    uint32_t sample = ((uint32_t)s_buff[i + 3] << 24) | ((uint32_t)s_buff[i + 2] << 16) | ((uint32_t)s_buff[i + 1] << 8) | (uint32_t)s_buff[i];
    dac_value = sample / 65537;
    d_buff[j++] = 0;
    d_buff[j++] = dac_value * 256 / 2048;
  }
}

void onEventsCallback(WebsocketsEvent event, String data) {
  if (event == WebsocketsEvent::ConnectionOpened) {
    Serial.println("WebSocket connection opened");
    internet_connect = true;
  } else if (event == WebsocketsEvent::ConnectionClosed) {
    Serial.println("WebSocket connection closed");
    internet_connect = false;
    // ESP.restart();
  }
}

void startI2S() {
  i2s_driver_install(I2S_PORT, &i2sMicConfig, 4, NULL);
  i2s_set_pin(I2S_PORT, &i2s_mic_pins);
  Serial.println("I2S started");
}

void stopI2S() {
  i2s_driver_uninstall(I2S_PORT);
  Serial.println("I2S stopped");
}

void Voice_Conversation::sendLink2ESP(void) {
  if (this->linkAudio.length() > 0) {
    Serial2.println(this->linkAudio);
    Serial.println("Gửi link qua UART: " + this->linkAudio);
  }
}
void Voice_Conversation::sendVoice2Server(const char *buff) {
  client.sendBinary(buff, I2S_READ_LEN);
  client.poll();
}


void voiceProcessTask(void *param)
{
  CommandDetector *commandDetector = static_cast<CommandDetector *>(param);
  const TickType_t xMaxBlockTime = pdMS_TO_TICKS(100);

  static int time_out = 0;
  
  while (true)
  {
    // wait for some audio samples to arrive
    uint32_t ulNotificationValue = ulTaskNotifyTake(pdTRUE, xMaxBlockTime);

    if(status_Robot == WAIT_INPUT)
    {
      if (ulNotificationValue > 0)
      {
          commandDetector->run();
      }
#if(ESP32_BUTTON)
      if(!push3s_Home())
      {
        voice_con.state = NO_READ;
        status_Robot = SLEEPING;
      } 
#endif 

#if(ESP32_INTERNET)
      if(!internet_connect)
      {
        voice_con.state = NO_READ;
        status_Robot = SLEEPING;
      } 
#endif

#if(ESP32_SENSOR)
      if(!check_sensor(SS1_PIN) || !check_sensor(SS2_PIN))
      {
        status_Robot = SENSOR_OFF;
      } 
#endif 
    }
    else if(status_Robot == ROBOT_ONLINE)
    {
      if(voice_con.state == GET_VOICE_CONVERSATION)
      {
        voice_con.sendVoice2Server((const char*)flash_write_buff);
#if(ESP32_BUTTON && ESP32_TIMEOUT)
        if(!check_timeout() || !push3s_Home())
        {
          voice_con.state = NO_READ;
          status_Robot = WAIT_INPUT;
          memset(flash_write_buff, 0, I2S_READ_LEN);
        }
#endif

#if(ESP32_INTERNET)
        if(!internet_connect)
        {
          voice_con.state = NO_READ;
          status_Robot = SLEEPING;
        } 
#endif
      }      
      else if(voice_con.state == SEND_VOICE_SERVER)
      {
        if (!voice_con.linkSent) {
          memset(flash_write_buff, 0, I2S_READ_LEN);
          status_Robot = ROBOT_RECEIVED_RESPONE;
          voice_con.linkSent = true;
          voice_con.sendLink2ESP();  
        }

#if(ESP32_TIMEOUT)
        if(!check_timeout())
        {
          voice_con.state = NO_READ;
          status_Robot = WAIT_INPUT;
        }
#endif

#if(ESP32_BUTTON)
        if(!push3s_Home())
        {
          voice_con.state = NO_READ;
          status_Robot = SLEEPING;
        }
#endif
      
#if(ESP32_INTERNET)
        if(!internet_connect)
        {
          voice_con.state = GET_VOICE_CONVERSATION;
          status_Robot = ROBOT_ONLINE;
        } 
#endif
      }
    }
    else if(status_Robot == SENSOR_OFF)
    {
#if(ESP32_BUTTON)
      if(!push3s_Home())
      {
        voice_con.state = NO_READ;
        status_Robot = SLEEPING;
      }
#endif

#if(ESP32_SENSOR)
      if(check_sensor(SS1_PIN) && check_sensor(SS2_PIN))
      {
        status_Robot = WAIT_INPUT;
      }
#endif
    }
    Serial.printf("status RB = %d, state_voice = %d\r\n", status_Robot, voice_con.state);
    vTaskDelay(pdMS_TO_TICKS(10)); 
  }
}
