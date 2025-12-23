#include "projdefs.h"
#include "mytasks.h"
#include <climits>

void vAlarmTask(void *pvParameters)
{
    PwmOut      buzzr(p26);
    AnalogIn    freqPot(p19);
    AnalogIn    volumePot(p20);

    EventBits_t prevBits = 0;

    for (;;) {

        EventBits_t currBits = xEventGroupGetBits(xAlarmEvents);
        EventBits_t newBits  = currBits & ~prevBits;

        if (newBits & ALARM_ACTIVE) {
            // Buzz for tala seconds
            TickType_t start = xTaskGetTickCount();
            TickType_t dur   = pdMS_TO_TICKS(tala * 1000);

            while (xTaskGetTickCount() - start < dur) {
                if (CS) {
                    buzzr.period(1.0f / (2000.0f + freqPot.read() * 16000.0f));
                    buzzr = volumePot.read();
                }
                else {
                    buzzr.period(1.0f/5000.0f);
                    buzzr = 0.8;
                }
                vTaskDelay(pdMS_TO_TICKS(50));
            }

            buzzr = 0;
        }

        // Only remember the bits that are still active
        prevBits = currBits;

        vTaskDelay(pdMS_TO_TICKS(50));  // small delay to yield
    }
}

