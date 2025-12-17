
#include "tasks.h"
#include "LM75B.h"
#include "lcd_driver.h"
#include "projdefs.h"
#include "rtc_driver.h"
#include <stdio.h>
#include <string.h>


QueueHandle_t xLcdQueue;
QueueHandle_t xRtcQueue;
SemaphoreHandle_t xRtcMutex;
SemaphoreHandle_t xI2CMutex;

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
    lcd_display_state_t current_state = LCD_STATE_FULL;

    // Keep track of last known values
    rtc_time_t  current_time = {0};
    float       current_temp = 0.0f;
    uint8_t     alarm_clock = 0, alarm_temp = 0;

    bool        time_dirty = false;
    bool        temp_dirty = false;
    bool        alarms_dirty = false;
    bool        state_dirty = false;

    char line1[32], line2[32], line3[32];

    for (;;) {
        // Wait indefinitely for a message
        if (xQueueReceive(xLcdQueue, &msg, portMAX_DELAY) == pdTRUE) {

            // Update internal state if needed
            switch (msg.type) {
                case LCD_MSG_SET_STATE:
                    current_state = msg.state;
                    state_dirty = true;
                    break;

                case LCD_MSG_UPDATE_TIME:
                    current_time = msg.time;
                    alarm_clock = msg.alarm_clock;
                    alarm_temp = msg.alarm_temp;
                    time_dirty = true;
                    alarms_dirty = true;
                    break;

                case LCD_MSG_UPDATE_TEMP:
                    current_temp = msg.temperature;
                    temp_dirty = true;
                    break;

                case LCD_MSG_SHOW_MESSAGE:
                    lcd_clear();
                    lcd_print(5, 10, msg.message);
                    lcd_update();
                    continue; // skip redraw of normal lines

                case LCD_MSG_CLEAR:
                    lcd_clear();
                    lcd_update();
                    continue; // skip redraw of normal lines

                case LCD_MSG_UPDATE_ALARMS:
                    // Future use
                    break;
            }

            // Redraw the LCD based on current state
            switch (current_state) {

                case LCD_STATE_FULL:
                    if (state_dirty) {
                        lcd_clear();
                        state_dirty = false;
                        time_dirty = temp_dirty = alarms_dirty = true;
                    }
                    if (time_dirty) {
                        sprintf(line1, "%02d:%02d:%02d",
                                current_time.hours,
                                current_time.minutes,
                                current_time.seconds);
                        lcd_print(0, 0, line1);
                        time_dirty = false;
                    }
                    if (alarms_dirty) {
                        sprintf(line2, "A:%c%c",
                                alarm_clock ? 'C' : ' ',
                                alarm_temp  ? 'T' : ' ');
                        lcd_print(0, 10, line2);
                        alarms_dirty = false;
                    }
                    if (temp_dirty) {
                        sprintf(line3, "T=%.1fC", current_temp);
                        lcd_print(50, 10, line3);
                        temp_dirty = false;
                    }
                    lcd_update();
                    break;
                default:
                    break;
            }
        }
        // vTaskDelay(pdMS_TO_TICKS(10));
    }
}


void vConsoleTask(void *pvParameters)
{
    for (;;) {
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

void vTempTask(void *pvParameters)
{
    LM75B lm75b(p28, p27);
    float temp;
    lcd_message_t msg;

    for (;;) {

        if (xSemaphoreTake(xI2CMutex, pdMS_TO_TICKS(50)) == pdTRUE) {

            temp = lm75b.read();
            xSemaphoreGive(xI2CMutex);
            msg.type = LCD_MSG_UPDATE_TEMP;
            msg.temperature = temp;
            xQueueSend(xLcdQueue, &msg, 0);
        }

        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

