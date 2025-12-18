#include "tasks.h"
#include "portmacro.h"
#include "LM75B.h"
#include "projdefs.h"
#include <climits>
#include <stdio.h>
#include <string.h>

void vTempTask(void *pvParameters)
{
    LM75B           lm75b(p28, p27);
    float           temp;
    lcd_message_t   msg;
    uint32_t        events;

    for (;;) {

        xTaskNotifyWait(0, ULONG_MAX, &events, portMAX_DELAY);

        if (xSemaphoreTake(xI2CMutex, pdMS_TO_TICKS(50)) == pdTRUE) {
            temp = lm75b.read();
            xSemaphoreGive(xI2CMutex);
        }

        if (events & TEMP_EVT_PERIODIC) {
            msg.type = LCD_MSG_UPDATE_TEMP;
            msg.temperature = temp;
            xQueueSend(xLcdQueue, &msg, 0);
        }

        if (events & TEMP_EVT_CMD) {
            
        }

    }
}
