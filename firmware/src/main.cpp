#include <Arduino.h>
#include <driver/i2s.h>
#include <esp_task_wdt.h>
#include "I2SMicSampler.h"
#include "I2SSampler.h"
#include "ADCSampler.h"
#include "CommandDetector.h"

#include "Peripheral/config.h"


#include "Wifi_config/wifi_config.h"
#include "Voice/voice.h"
#include "Cert/cert.h"
#include "FSM/fsm.h"



CommandDetector *commandDetector;

void setup()
{
  Serial.begin(115200);
  delay(1000);
  Serial.println("Starting up");
  xSemaphore = xSemaphoreCreateMutex();
#if(ESP32_WIFI_CONFIG)
  connect_wifi_server(); //config wifi on web server
  while(status_Robot == WAITING_WIFI_CONNECT)
  {
      if(nvs_config())
      {
          status_Robot = WAIT_INPUT;
      }
      delay(10);
  }
#endif
  // make sure we don't get killed for our long running tasks
  // esp_task_wdt_init(10, false);
  // disableCore0WDT();  // Disable watchdog for core 0
  // disableCore1WDT();  // Disable watchdog for core 1
#if(ESP32_VOICE_CONVERSATION)
  // i2s_read_buff = (char*)calloc(I2S_READ_LEN, sizeof(char));
  flash_write_buff = (uint8_t*)calloc(I2S_READ_LEN, sizeof(char));

  client.onEvent(onEventsCallback);
  client.onMessage(onMessageCallback);
  client.setCACert(ssl_cert);
  while (!client.connect("wss://tofutest.stepup.edu.vn/tofu/ws/conversation")) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("WebSocket connected!");
  // TaskHandle_t voiceConversationTaskHandle;
  // xTaskCreatePinnedToCore(voiceConversationTask, "Conversation", 8192, NULL, 1, &voiceConversationTaskHandle, 0);
#endif

#if(ESP32_VOICE_WAKEUP)
  // the command processor
  CommandProcessor *command_processor = new CommandProcessor();

  // // create our application
  CommandDetector *commandDetector = new CommandDetector(i2s_sampler, command_processor);

  // // set up the i2s sample writer task
  
  xTaskCreatePinnedToCore(voiceWakeupTask, "Wakeup", 4096, commandDetector, 3, &voiceWakeupTaskHandle, 0);
  i2s_sampler->start(I2S_NUM_0, i2sMicConfig, voiceWakeupTaskHandle);
  
#endif  
}

void loop()
{
  // commandDetector->run();
  vTaskDelay(pdMS_TO_TICKS(10));
  // Serial.printf("status robot run = %d\r\n", status_Robot);
}