
#include "tasks.h"
#include "lcd_driver.h"
#include "rtc_driver.h"
#include <stdio.h>
#include <string.h>


QueueHandle_t xLcdQueue;
QueueHandle_t xRtcQueue;
SemaphoreHandle_t xRtcMutex;


void vRtcTask(void *pvParameters)
{
    rtc_command_t cmd;
    rtc_time_t current_time;
    lcd_message_t lcd_msg;
    TickType_t xLastWakeTime;
    
    xLastWakeTime = xTaskGetTickCount();
    
    for (;;) {
        // Check for commands (non-blocking)
        if (xQueueReceive(xRtcQueue, &cmd, 0) == pdTRUE) {
            
            xSemaphoreTake(xRtcMutex, portMAX_DELAY);
            
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
            
            xSemaphoreGive(xRtcMutex);
        }
        
        // Periodic: Read RTC and send to LCD (every second)
        xSemaphoreTake(xRtcMutex, portMAX_DELAY);
        rtc_get_time(&current_time);
        xSemaphoreGive(xRtcMutex);
        
        // Send time update to LCD
        lcd_msg.type = LCD_MSG_UPDATE_TIME;
        lcd_msg.time = current_time;
        xQueueSend(xLcdQueue, &lcd_msg, 0);
        
        // Wait 1 second
        vTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(1000));
    }
}


void vLcdTask(void *pvParameters)
{
    lcd_message_t msg;
    lcd_display_state_t current_state = LCD_STATE_TIME;
    char line1[32], line2[32], line3[32];
    
    for (;;) {
        // Wait for message (blocking)
        if (xQueueReceive(xLcdQueue, &msg, portMAX_DELAY) == pdTRUE) {
            
            switch (msg.type) {
                case LCD_MSG_SET_STATE:
                    current_state = msg.state;
                    break;
                    
                case LCD_MSG_UPDATE_TIME:
                    switch (current_state) {
                        case LCD_STATE_TIME:
                            // Simple time display
                            sprintf(line1, "RTC System");
                            sprintf(line2, "%02d:%02d:%02d", 
                                    msg.time.hours, msg.time.minutes, msg.time.seconds);
                            sprintf(line3, "%02d/%02d/%04d",
                                    msg.time.day, msg.time.month, msg.time.year);
                            
                            lcd_clear();
                            lcd_print(5, 0, line1);
                            lcd_print(5, 10, line2);
                            lcd_print(5, 20, line3);
                            lcd_update();
                            break;
                            
                        case LCD_STATE_FULL:
                            // Full display (future)
                            sprintf(line1, "%02d:%02d:%02d", 
                                    msg.time.hours, msg.time.minutes, msg.time.seconds);
                            sprintf(line2, "A:%c%c", 
                                    msg.alarm_clock ? 'C' : ' ',
                                    msg.alarm_temp ? 'T' : ' ');
                            sprintf(line3, "T=%.1fC", msg.temperature);
                            
                            lcd_clear();
                            lcd_print(0, 0, line1);
                            lcd_print(0, 10, line2);
                            lcd_print(50, 10, line3);
                            lcd_update();
                            break;
                            
                        case LCD_STATE_BUBBLE_LEVEL:
                        case LCD_STATE_MESSAGE:
                            break;
                    }
                    break;
                    
                case LCD_MSG_SHOW_MESSAGE:
                    lcd_clear();
                    lcd_print(5, 10, msg.message);
                    lcd_update();
                    break;
                    
                case LCD_MSG_CLEAR:
                    lcd_clear();
                    lcd_update();
                    break;
                    
                case LCD_MSG_UPDATE_TEMP:
                case LCD_MSG_UPDATE_ALARMS:
                    // Future use
                    break;
            }
        }
    }
}


void vConsoleTask(void *pvParameters)
{
    for (;;) {
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}