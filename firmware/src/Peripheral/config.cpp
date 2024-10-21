#include "config.h"

int status_Robot = 0; //WAITING_WIFI_CONNECT

void LED_ON(void)
{
    digitalWrite(LED_PIN, HIGH);
}

void LED_OFF(void)
{
    digitalWrite(LED_PIN, LOW);
}

void MIC_WAKEUP(void)
{
    digitalWrite(L_R_MIC_PIN, LOW);
}

void MIC_CONVERSATION(void)
{
    digitalWrite(L_R_MIC_PIN, HIGH);
}




