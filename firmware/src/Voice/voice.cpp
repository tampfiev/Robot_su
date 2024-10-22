#include "voice.h"

TaskHandle_t voiceWakeupTaskHandle;
TaskHandle_t voiceConversationTaskHandle;
I2SSampler *i2s_sampler = new I2SMicSampler(i2s_mic_pins, false);

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


// void i2s_adc_task() {
//   size_t bytes_read;
//   i2s_read(I2S_PORT, (void*)i2s_read_buff, I2S_READ_LEN, &bytes_read, portMAX_DELAY);
//   i2s_adc_data_scale(flash_write_buff, (uint8_t*)i2s_read_buff, I2S_READ_LEN);
//   client.sendBinary((const char*)flash_write_buff, I2S_READ_LEN);
//   client.poll();
// }

void i2s_adc_convert(uint8_t *i2sData) {
  i2s_adc_data_scale(flash_write_buff, (uint8_t*)i2sData, I2S_READ_LEN);
  client.sendBinary((const char*)flash_write_buff, I2S_READ_LEN);
  client.poll();
}

void i2s_adc_task() {
  size_t bytes_read;
  i2s_read(I2S_PORT, (void*)i2s_read_buff, I2S_READ_LEN, &bytes_read, portMAX_DELAY);
  i2s_adc_data_scale(flash_write_buff, (uint8_t*)i2s_read_buff, I2S_READ_LEN);
  client.sendBinary((const char*)flash_write_buff, I2S_READ_LEN);
}

void sendLinkToESP2() {
  if (linkAudioToSpeech.length() > 0) {
    Serial2.println(linkAudioToSpeech);
    Serial.println("Gửi link qua UART: " + linkAudioToSpeech);
  }
}

void onMessageCallback(WebsocketsMessage message) {
  String respond_message = message.data();
  Serial.println(respond_message);

  if (respond_message.indexOf("\"audio\":") != -1) {
    int indexOfText = respond_message.indexOf("\"text\":");
    int indexOfAudio = respond_message.indexOf("\"audio\":");
    int indexOfEmotion = respond_message.indexOf("\"emotion\":");
    String textToSpeech = respond_message.substring(indexOfText + 8, indexOfAudio - 2);
    linkAudioToSpeech = respond_message.substring(indexOfAudio + 9, indexOfEmotion - 2);
    Serial.println("Text: " + textToSpeech);
    Serial.println("Audio Link: " + linkAudioToSpeech);

    currentState = 2;
    linkSent = false;
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
  } else if (event == WebsocketsEvent::ConnectionClosed) {
    Serial.println("WebSocket connection closed");
    ESP.restart();
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


void voiceWakeupTask(void *param)
{
  CommandDetector *commandDetector = static_cast<CommandDetector *>(param);
  const TickType_t xMaxBlockTime = pdMS_TO_TICKS(100);
  
  while (true)
  {
    // wait for some audio samples to arrive
    uint32_t ulNotificationValue = ulTaskNotifyTake(pdTRUE, xMaxBlockTime);
    // Serial.printf("current State: %d, flag_I2S = %d, status = %d\r\n", currentState, flag_I2S, status_Robot);
    // if (ulNotificationValue > 0)
    // {
    //   // if(status_Robot == WAIT_INPUT)
    //   // {
    //   //   MIC_WAKEUP();
    //       commandDetector->run();
  
    //   // }     
    // }
    if(currentState == 1)
    {
      i2s_adc_task();
      client.poll();
      // flag_I2S = false;
    }
    // else if((status_Robot == ROBOT_ONLINE) && 
    // if(flag_I2S == true)
    // {
    // if((currentState == 1) && (flag_I2S))
    // {
    //   i2s_adc_task();
    //   flag_I2S = false;
    // }
      
    if(currentState == 2)
    {
      if (!linkSent) {
        memset(i2s_read_buff, 0, I2S_READ_LEN);
        memset(flash_write_buff, 0, I2S_READ_LEN);
        stopI2S();
        // i2s_sampler->stop();
        sendLinkToESP2();
        linkSent = true;
      }
    }
    // Serial.printf("current State: %d, flag_I2S = %d, status = %d\r\n", currentState, flag_I2S, status_Robot);
    // }
    // Serial.printf("current State: %d\r\n", currentState);
    // Serial.printf("status robot run wait input= %d\r\n", status_Robot);
    vTaskDelay(pdMS_TO_TICKS(10)); 
  }
}


void voiceConversationTask(void *param)
{
  const TickType_t xMaxBlockTime = pdMS_TO_TICKS(100);
  i2s_sampler->start(I2S_NUM_0, i2sMicConfig, voiceConversationTaskHandle);
  while (true)
  {
    // wait for some audio samples to arrive
    if(status_Robot == ROBOT_ONLINE)
    {
      MIC_CONVERSATION();
      Serial.println("Start Conversation");
      i2s_adc_task();
      client.poll();
    }
    else if(currentState == 3)
    {
      Serial.println("Start Send Url");
      if (!linkSent) {
        // memset(i2s_read_buff, 0, I2S_READ_LEN);
        memset(flash_write_buff, 0, I2S_READ_LEN);
        i2s_sampler->stop();
        sendLinkToESP2();
        linkSent = true;
      }
    }
    Serial.printf("status robot run conversation = %d\r\n", status_Robot);
    vTaskDelay(pdMS_TO_TICKS(10)); 
  }
}