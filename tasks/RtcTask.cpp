#include "config.h"
#include "mytasks.h"
#include "projdefs.h"
#include "rtc_driver.h"
#include <stdio.h>
#include <string.h>

void vRtcTask(void *pvParameters)
{
    rtc_time_t      current_time;
    rtc_time_t      alarm_time;
    rtc_message_t   rtc_msg;
    lcd_message_t   lcd_msg;
    TickType_t      xLastWakeTime;
    TickType_t      lastLcdUpdate = 0;   // timestamp of last LCD update
    
    const TickType_t taskPeriod = pdMS_TO_TICKS(50);   // loop period 50 ms
    const TickType_t lcdPeriod  = pdMS_TO_TICKS(1000); // LCD update every 1 s

    xLastWakeTime = xTaskGetTickCount();
    rtc_start();

    for (;;) {
        // Get current time
        rtc_get_time(&current_time);

        // 1. Non-blocking check for commands
        while (xQueueReceive(xRtcQueue, &rtc_msg, 0) == pdTRUE) {    
            switch (rtc_msg.type) {
                case RTC_CMD_SET_TIME:
                    rtc_msg.time.year  = current_time.year;
                    rtc_msg.time.month = current_time.month;
                    rtc_msg.time.day   = current_time.day;
                    rtc_set_time(&(rtc_msg.time));
                    break;
                
                case RTC_CMD_SET_DATE:
                    rtc_msg.time.hours   = current_time.hours;
                    rtc_msg.time.minutes = current_time.minutes;
                    rtc_msg.time.seconds = current_time.seconds;
                    rtc_set_time(&(rtc_msg.time));
                    break;
                    
                case RTC_CMD_READ_DATETIME: {
                    rtc_message_t response;
                    response.type = RTC_CMD_RESPONSE;
                    rtc_get_time(&response.time);
                    xQueueSend(xConsoleQueue, &response, 0);
                    break;
                }
            }
        }

        // 2. Update LCD every 1 second
        TickType_t now = xTaskGetTickCount();
        if ((now - lastLcdUpdate) >= lcdPeriod) {
            lcd_msg.type = LCD_MSG_UPDATE_TIME;
            lcd_msg.data.time = current_time;
            xQueueSend(xLcdQueue, &lcd_msg, 0);
            lastLcdUpdate = now;
        }

        // 3. Check alarm
        if (ALAC) {
            alarm_time.seconds = ALAS;
            alarm_time.minutes = ALAM;
            alarm_time.hours   = ALAH;
            alarm_time.day     = current_time.day;
            alarm_time.month   = current_time.month;
            alarm_time.year    = current_time.year;

            if (rtc_time_diff(current_time, alarm_time) == 0) {
                xEventGroupSetBits(xAlarmEvents, ALARM_TIME);
            }
        }

        // 4. Wait until next iteration
        vTaskDelayUntil(&xLastWakeTime, taskPeriod);
    }
}
