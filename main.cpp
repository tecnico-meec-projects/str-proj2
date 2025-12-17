
#include "mbed.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"
#include "LCD/lcd_driver.h"
#include "RTC/rtc_driver.h"
#include "tasks/tasks.h"


int main(void)
{

    xLcdQueue = xQueueCreate(5, sizeof(lcd_message_t));
    xRtcQueue = xQueueCreate(5, sizeof(rtc_command_t));
    xRtcMutex = xSemaphoreCreateMutex();
    xI2CMutex = xSemaphoreCreateMutex();
    

    xTaskCreate(vRtcTask, "RTC", 256, NULL, 3, NULL);
    xTaskCreate(vLcdTask, "LCD", 256, NULL, 2, NULL);
    // xTaskCreate(vConsoleTask, "Console", 256, NULL, 1, NULL);
    xTaskCreate(vTempTask, "Temperature Monitoring", 256, NULL, 3, NULL);
    

    vTaskStartScheduler();

    for (;;);
    return 0;
}
