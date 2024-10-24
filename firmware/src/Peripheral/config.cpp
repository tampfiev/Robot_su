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

int read_BTN(const int _pin)
{
    return digitalRead(_pin);
}

bool push3s_Home(void)
{
    static int cnt_time = 0;

    if(read_BTN(HOME_PIN) == PUSH_BUTTON)
    {
        cnt_time++;
        if(cnt_time > TIME_OUT_3S)
        {
            cnt_time = 0;
            return true;
        }
    }
    else
    {
        cnt_time = 0;
    }
    return false;
}

bool check_timeout(void)
{
    static int cnt_time = 0;
    cnt_time++;
    if(cnt_time > TIME_OUT_3S)
    {
        cnt_time = 0;
        return true;
    }
    return false;
}






