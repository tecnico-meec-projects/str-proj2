#include "tasks.h"
#include "projdefs.h"
#include "rtc_driver.h"
#include <stdio.h>
#include <string.h>

void vRtcTask(void *pvParameters)
{
    rtc_command_t   cmd;
    rtc_time_t      current_time;
    lcd_message_t   lcd_msg;
    TickType_t      xLastWakeTime;
    
    xLastWakeTime = xTaskGetTickCount();
    rtc_start();
    for (;;) {
        // Check for commands (non-blocking)
        if (xQueueReceive(xRtcQueue, &cmd, 0) == pdTRUE) {
            
            // xSemaphoreTake(xRtcMutex, portMAX_DELAY);
            
            switch (cmd.type) {
                case RTC_CMD_SET_TIME:
                case RTC_CMD_SET_DATE:
                    rtc_set_time(&cmd.time);
                    break;
                    
                case RTC_CMD_READ_TIME:
                case RTC_CMD_READ_DATETIME:
                    rtc_get_time(&current_time);
                    
                    if (cmd.response_queue != NULL) {
                        rtc_response_t response;
                        response.time = current_time;
                        response.success = 1;
                        xQueueSend(cmd.response_queue, &response, 0);
                    }
                    break;
            }
            
            // xSemaphoreGive(xRtcMutex);
        }
        
        // Periodic: Read RTC and send to LCD (every second)
        // xSemaphoreTake(xRtcMutex, portMAX_DELAY);
        rtc_get_time(&current_time);
        // xSemaphoreGive(xRtcMutex);
        
        // Send time update to LCD
        lcd_msg.type = LCD_MSG_UPDATE_TIME;
        lcd_msg.time = current_time;
        xQueueSend(xLcdQueue, &lcd_msg, 0);
        
        // Wait 1 second
        vTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(1000));
    }
}