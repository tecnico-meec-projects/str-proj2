
#include "mbed.h"
#include "FreeRTOS.h"
#include "projdefs.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"
#include "event_groups.h"
#include "LCD/lcd_driver.h"
#include "RTC/rtc_driver.h"
#include "tasks/tasks.h"


// Queues
QueueHandle_t       xLcdQueue;
QueueHandle_t       xRtcQueue;
QueueHandle_t       xRtcResponseQueue;

// Mutexes
SemaphoreHandle_t   xRtcMutex;
SemaphoreHandle_t   xI2CMutex;

// Event groups
EventGroupHandle_t  xAlarmEvents;

// Software timers
TimerHandle_t       xTempTimer;

// Tasks
TaskHandle_t        xTempTaskHandle;


void vTempTimerCallback(TimerHandle_t xTimer){
    xTaskNotify(xTempTaskHandle, TEMP_EVT_PERIODIC, eSetBits);
}

int main(void)
{

    xLcdQueue = xQueueCreate(100, sizeof(lcd_message_t));
    xRtcQueue = xQueueCreate(5, sizeof(rtc_command_t));
    xRtcResponseQueue = xQueueCreate(1, sizeof(rtc_response_t));
    xRtcMutex = xSemaphoreCreateMutex();
    xI2CMutex = xSemaphoreCreateMutex();
    xTempTimer = xTimerCreate("Temp timer", pdMS_TO_TICKS(1000), pdTRUE, NULL, vTempTimerCallback);
    xTimerStart(xTempTimer, 0);

    xTaskCreate(vRtcTask, "RTC", 256, NULL, 3, NULL);
    xTaskCreate(vLcdTask, "LCD", 256, NULL, 2, NULL);
    xTaskCreate(vConsoleTask, "Console", 256, NULL, 1, NULL);
    xTaskCreate(vTempTask, "Temperature Monitoring", 256, NULL, 3, &xTempTaskHandle);
    xTaskCreate(vBubLvlTask, "Bubble Level", 256, NULL, 3, NULL);
    xTaskCreate(vAlarmTask, "Alarm", 256, NULL, 3, NULL);

    vTaskStartScheduler();

    for (;;);
    return 0;
}
