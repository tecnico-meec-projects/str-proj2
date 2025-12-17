
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include "tasks.h"
#include "rtc_driver.h"

void cmd_rc(int argc, char **argv)
{
    rtc_command_t cmd;
    rtc_response_t response;
    QueueHandle_t response_queue;
    
    response_queue = xQueueCreate(1, sizeof(rtc_response_t));
    if (!response_queue) { 
        printf("Error: Failed to create response queue\n"); 
        return; 
    }
    
    cmd.type = RTC_CMD_READ_TIME;
    cmd.response_queue = response_queue;
    
    if (xQueueSend(xRtcQueue, &cmd, pdMS_TO_TICKS(100)) != pdTRUE) {
        printf("Error: Failed to send command\n");
        vQueueDelete(response_queue);
        return;
    }
    
    if (xQueueReceive(response_queue, &response, pdMS_TO_TICKS(1000)) == pdTRUE) {
        printf("Clock: %02d:%02d:%02d\n", 
               response.time.hours, response.time.minutes, response.time.seconds);
    } else {
        printf("Error: Timeout\n");
    }
    
    vQueueDelete(response_queue);
}


void cmd_rdt(int argc, char **argv)
{
    rtc_command_t cmd;
    rtc_response_t response;
    QueueHandle_t response_queue;
    
    response_queue = xQueueCreate(1, sizeof(rtc_response_t));
    if (!response_queue) { 
        printf("Error: Failed to create response queue\n"); 
        return; 
    }
    
    cmd.type = RTC_CMD_READ_DATETIME;
    cmd.response_queue = response_queue;
    
    if (xQueueSend(xRtcQueue, &cmd, pdMS_TO_TICKS(100)) != pdTRUE) {
        printf("Error: Failed to send command\n");
        vQueueDelete(response_queue);
        return;
    }
    
    if (xQueueReceive(response_queue, &response, pdMS_TO_TICKS(1000)) == pdTRUE) {
        printf("Date/Time: %02d/%02d/%04d %02d:%02d:%02d\n",
               response.time.day, response.time.month, response.time.year,
               response.time.hours, response.time.minutes, response.time.seconds);
    } else {
        printf("Error: Timeout\n");
    }
    
    vQueueDelete(response_queue);
}


void cmd_sc(int argc, char **argv)
{
    if (argc != 4) { 
        printf("Usage: sc <hours> <minutes> <seconds>\n"); 
        return; 
    }
    
    rtc_command_t cmd;
    rtc_response_t response;
    QueueHandle_t response_queue;
    
    // Read current date first
    response_queue = xQueueCreate(1, sizeof(rtc_response_t));
    cmd.type = RTC_CMD_READ_DATETIME;
    cmd.response_queue = response_queue;
    xQueueSend(xRtcQueue, &cmd, pdMS_TO_TICKS(100));
    xQueueReceive(response_queue, &response, pdMS_TO_TICKS(1000));
    vQueueDelete(response_queue);
    
    // Set new time
    cmd.type = RTC_CMD_SET_TIME;
    cmd.time = response.time;
    cmd.time.hours = atoi(argv[1]);
    cmd.time.minutes = atoi(argv[2]);
    cmd.time.seconds = atoi(argv[3]);
    cmd.response_queue = NULL;
    
    if (xQueueSend(xRtcQueue, &cmd, pdMS_TO_TICKS(100)) == pdTRUE) {
        printf("Clock set to %02d:%02d:%02d\n", 
               cmd.time.hours, cmd.time.minutes, cmd.time.seconds);
    } else {
        printf("Error: Failed to set clock\n");
    }
}


void cmd_sd(int argc, char **argv)
{
    if (argc != 4) { 
        printf("Usage: sd <day> <month> <year>\n"); 
        return; 
    }
    
    rtc_command_t cmd;
    rtc_response_t response;
    QueueHandle_t response_queue;
    
    // Read current time first
    response_queue = xQueueCreate(1, sizeof(rtc_response_t));
    cmd.type = RTC_CMD_READ_DATETIME;
    cmd.response_queue = response_queue;
    xQueueSend(xRtcQueue, &cmd, pdMS_TO_TICKS(100));
    xQueueReceive(response_queue, &response, pdMS_TO_TICKS(1000));
    vQueueDelete(response_queue);
    
    // Set new date
    cmd.type = RTC_CMD_SET_DATE;
    cmd.time = response.time;
    cmd.time.day = atoi(argv[1]);
    cmd.time.month = atoi(argv[2]);
    cmd.time.year = atoi(argv[3]);
    cmd.response_queue = NULL;
    
    if (xQueueSend(xRtcQueue, &cmd, pdMS_TO_TICKS(100)) == pdTRUE) {
        printf("Date set to %02d/%02d/%04d\n", 
               cmd.time.day, cmd.time.month, cmd.time.year);
    } else {
        printf("Error: Failed to set date\n");
    }
}