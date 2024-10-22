// #include <Arduino.h>
#include "I2SSampler.h"
// #include <driver/i2s.h>
// #include <algorithm>
// #include "RingBuffer.h"
// #include "../../src/Peripheral/config.h"
// #include "../../src/Voice/voice.h"
// #include "../../src/Wifi_config/wifi_config.h"


bool flag_I2S = false;

void I2SSampler::addSample(int16_t sample)
{
    // store the sample
    m_write_ring_buffer_accessor->setCurrentSample(sample);
    if (m_write_ring_buffer_accessor->moveToNextSample())
    {
        // trigger the processor task as we've filled a buffer
        xTaskNotify(m_processor_task_handle, 1, eSetBits);
    }
}

void i2sReaderTask(void *param)
{
    I2SSampler *sampler = (I2SSampler *)param;
    char *i2sData;
    i2sData = (char*)calloc(1024, sizeof(char));
    while (true)
    {
        // wait for some data to arrive on the queue
        i2s_event_t evt;
        if (xQueueReceive(sampler->m_i2s_queue, &evt, portMAX_DELAY) == pdPASS)
        {
            if (evt.type == I2S_EVENT_RX_DONE)
            {
                size_t bytesRead = 0;
                do
                {
                    // read data from the I2S peripheral
                    // uint8_t i2sData[8192];
                    i2s_read(sampler->getI2SPort(), (void*) i2sData, 1024, &bytesRead, portMAX_DELAY);
                    // process the raw data
                    // if(status_Robot == 2) //WAITING INPUT
                    // {
                        sampler->processI2SData((uint8_t*)i2sData, bytesRead);
                    // }
                    if(status_Robot == 3) //ROBOT_ONLINE
                    {
                        sampler->processI2SData_scale((uint8_t*)i2sData, flash_write_buff, 1024);
                        flag_I2S = true;
                        // if(currentState == 1)
                        // {
                        //   i2s_adc_task();
                        // }
                        // else if(currentState == 2)
                        // {
                        //   if (!linkSent) {
                        //     memset(flash_write_buff, 0, 1024);
                        //     // i2s_sampler->stop();
                        //     sendLinkToESP2();
                        //     linkSent = true;
                        //   }
                        // }                      
                    }
                } while (bytesRead > 0);
            }
        }
        vTaskDelay(pdMS_TO_TICKS(10)); 
    }
}

I2SSampler::I2SSampler()
{
    m_audio_buffers = (AudioBuffer **)malloc(sizeof(AudioBuffer **) * AUDIO_BUFFER_COUNT);

    // allocate the audio buffers
    for (int i = 0; i < AUDIO_BUFFER_COUNT; i++)
    {
        m_audio_buffers[i] = new AudioBuffer();
    }
    m_write_ring_buffer_accessor = new RingBufferAccessor(m_audio_buffers, AUDIO_BUFFER_COUNT);
}

void I2SSampler::start(i2s_port_t i2s_port, i2s_config_t &i2s_config, TaskHandle_t processor_task_handle)
{
    Serial.println("Starting i2s");
    m_i2s_port = i2s_port;
    m_processor_task_handle = processor_task_handle;
    //install and start i2s driver
    i2s_driver_install(m_i2s_port, &i2s_config, 0, NULL);
    // i2s_driver_install(m_i2s_port, &i2s_config, 4, &m_i2s_queue);
    // set up the I2S configuration from the subclass
    configureI2S();
    // start a task to read samples
    // xTaskCreatePinnedToCore(i2sReaderTask, "i2s Reader Task", 4096, this, 1, &m_reader_task_handle, 1);
}

void I2SSampler::stop()
{
    // Check if the reader task exists and delete it
    if (m_reader_task_handle != nullptr)
    {
        // Delete the task that reads I2S samples
        vTaskDelete(m_reader_task_handle);
        m_reader_task_handle = nullptr; // Set handle to null to avoid reuse
    }

    // Check if the I2S driver is installed and uninstall it
    if (m_i2s_port != I2S_NUM_MAX)  // Ensure the I2S port is valid
    {
        // Uninstall the I2S driver
        i2s_driver_uninstall(m_i2s_port);
        m_i2s_port = I2S_NUM_MAX; // Reset port to invalid value
    }

    // Nullify the queue handle as well for safety
    m_i2s_queue = nullptr;
    
    Serial.println("I2S stopped");
}


RingBufferAccessor *I2SSampler::getRingBufferReader()
{
    RingBufferAccessor *reader = new RingBufferAccessor(m_audio_buffers, AUDIO_BUFFER_COUNT);
    // place the reaader at the same position as the writer - clients can move it around as required
    reader->setIndex(m_write_ring_buffer_accessor->getIndex());
    return reader;
}
