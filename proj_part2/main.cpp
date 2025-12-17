
#include "mbed.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"
#include "lcd_driver.h"
#include "rtc_driver.h"
#include "tasks.h"


int main(void)
{
    // Initialize hardware
    lcd_init();
    rtc_start();
    
    // Show startup message
    lcd_clear();
    lcd_print(10, 5, "RTC-LCD System");
    lcd_print(10, 15, "Starting...");
    vTaskDelay(pdMS_TO_TICKS(2000));
    

    xLcdQueue = xQueueCreate(5, sizeof(lcd_message_t));
    xRtcQueue = xQueueCreate(5, sizeof(rtc_command_t));
    xRtcMutex = xSemaphoreCreateMutex();
    

    xTaskCreate(vRtcTask, "RTC", 256, NULL, 3, NULL);
    xTaskCreate(vLcdTask, "LCD", 256, NULL, 2, NULL);
    xTaskCreate(vConsoleTask, "Console", 256, NULL, 1, NULL);
    

    vTaskStartScheduler();

    for (;;);
    return 0;
}
