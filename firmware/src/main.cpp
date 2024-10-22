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
  // connect_wifi_server(); //config wifi on web server
  // while(status_Robot == WAITING_WIFI_CONNECT)
  // {
  //     if(nvs_config())
  //     {
  //         status_Robot = ROBOT_ONLINE;
  //     }
  //     delay(10);
  // }

  const char* ssid = "1805";
  const char* password = "123456a@";

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("WiFi connected");
#endif
  // make sure we don't get killed for our long running tasks
  // esp_task_wdt_init(10, false);
  // disableCore0WDT();  // Disable watchdog for core 0
  // disableCore1WDT();  // Disable watchdog for core 1
  // startI2S();
#if(ESP32_VOICE_CONVERSATION)
  i2s_read_buff = (char*)calloc(I2S_READ_LEN, sizeof(char));
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
  // xTaskCreatePinnedToCore(voiceConversationTask, "Conversation", 4096, NULL, 1, &voiceConversationTaskHandle, 0);
#endif

#if(ESP32_VOICE_WAKEUP)
  // the command processor
  CommandProcessor *command_processor = new CommandProcessor();

  // // create our application
  CommandDetector *commandDetector = new CommandDetector(i2s_sampler, command_processor);

  // // set up the i2s sample writer task
  
  xTaskCreatePinnedToCore(voiceWakeupTask, "Wakeup", 8192, commandDetector, 1, &voiceWakeupTaskHandle, 0);
  i2s_sampler->start(I2S_NUM_0, i2sMicConfig, voiceWakeupTaskHandle);
  
#endif  
}

void loop()
{
  // commandDetector->run();
  vTaskDelay(pdMS_TO_TICKS(10));
  // Serial.printf("status robot run = %d\r\n", status_Robot);
  // switch (currentState) {
  //   case 1:
  //     i2s_adc_task();
  //     client.poll();
  //     break;

  //   case 2:
  //     if (!linkSent) {
  //       memset(i2s_read_buff, 0, I2S_READ_LEN);
  //       memset(flash_write_buff, 0, I2S_READ_LEN);
  //       stopI2S();  // Dừng I2S khi chuyển sang trạng thái 2
  //       sendLinkToESP2();
  //       linkSent = true;
  //     }
  //     if (Serial2.available()) {
  //       char response = Serial2.read();
  //       if (response == '1') {
  //         Serial.println("Received '1' from ESP2, chuyển sang trạng thái 1");
  //         currentState = 1;
  //         linkSent = false;
  //         startI2S();  // Khởi động lại I2S khi quay lại trạng thái 1
  //         Serial.println(currentState);
  //         Serial.println(linkSent);
  //         delay(10);
  //       }
  //     }
  //     break;
  // }
}