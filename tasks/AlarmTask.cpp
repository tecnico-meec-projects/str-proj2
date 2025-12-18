#include "tasks.h"
#include <climits>


void vAlarmTask(void *vParameters)
{
    PwmOut      buzzr(p26);
    AnalogIn    freqPot(p19);
    AnalogIn    volumePot(p20);

    uint32_t    notify;
    bool        alarmActive = false;

    for (;;) {
        // xTaskNotifyWait(0, ULONG_MAX, &notify, portMAX_DELAY);

        buzzr.period(1.0/(2000 + freqPot.read()*16000));
        buzzr = volumePot.read();
        vTaskDelay(100);
    }
}