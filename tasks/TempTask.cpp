#include "mytasks.h"
#include "records.h"
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
    lcd_message_t   lcd_msg;
    temp_message_t  temp_msg;
    uint32_t        events;

    for (;;) {

        if (xSemaphoreTake(xI2CMutex, pdMS_TO_TICKS(50)) == pdTRUE)
        {
            temp = lm75b.read();
            xSemaphoreGive(xI2CMutex);
        }

        if (temp < min_temp)
            min_temp = temp;
        if (temp > max_temp)
            max_temp = temp;

        uint32_t raw;
        memcpy(&raw, &temp, sizeof(float));
        xTaskNotify(xRgbTaskHandle, raw, eSetValueWithOverwrite);

        if (ALAT)
        {
            if (temp < (float)(tlow) || temp > (float)(thigh))
                xEventGroupSetBits(xAlarmEvents, ALARM_TEMP);
        }

        xTaskNotifyWait(0, ULONG_MAX, &events, 0);

        if (events & TEMP_EVT_PERIODIC) {
            lcd_msg.type = LCD_MSG_UPDATE_TEMP;
            lcd_msg.data.temperature = temp;
            xQueueSend(xLcdQueue, &lcd_msg, 0);
        }

        if (events & TEMP_EVT_CMD) {
            temp_msg.type = TEMP_CMD_READ_TEMP;
            temp_msg.temp = temp;
            xQueueSend(xConsoleQueue, &temp_msg, 0);
        }

        vTaskDelay(pdMS_TO_TICKS(50));

    }
}
