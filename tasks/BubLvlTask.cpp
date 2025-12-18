
#include "tasks.h"
#include "imu_driver.h"
#include "projdefs.h"
#include <stdio.h>
#include <string.h>

void vBubLvlTask(void *vParameters)
{
    imu_init();
    struct tilt_t tilt;
    lcd_message_t msg;

    for (;;) {

        if (xSemaphoreTake(xI2CMutex, pdMS_TO_TICKS(50)) == pdTRUE) {

            tilt = imu_read();
            xSemaphoreGive(xI2CMutex);
            msg.type = LCD_MSG_UPDATE_BUBBLE;
            msg.tilt = tilt;
            xQueueSend(xLcdQueue, &msg, 0);
        }

        vTaskDelay(pdMS_TO_TICKS(20));
    }
}