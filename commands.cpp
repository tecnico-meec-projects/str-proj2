
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include "tasks.h"
#include "rtc_driver.h"

extern QueueHandle_t xRtcResponseQueue;

void cmd_test (int argc, char** argv)
{
  int i;

  /* exemplo -- escreve argumentos */
  for (i=0; i<argc; i++)
    printf ("\nargv[%d] = %s", i, argv[i]);
}

void cmd_rdt(int argc, char **argv)
{
    rtc_command_t cmd;
    rtc_response_t response;
    
    cmd.type = RTC_CMD_READ_DATETIME;
    cmd.response_queue = xRtcResponseQueue;

    if (xQueueSend(xRtcQueue, &cmd, pdMS_TO_TICKS(100)) != pdTRUE) {
        printf("Error: Failed to send command\n");
        return;
    }
    
    if (xQueueReceive(xRtcResponseQueue, &response, pdMS_TO_TICKS(1000)) == pdTRUE) {
        printf("Date/Time: %02d/%02d/%04d %02d:%02d:%02d\n",
               response.time.day, response.time.month, response.time.year,
               response.time.hours, response.time.minutes, response.time.seconds);
    } else {
        printf("Error: Timeout\n");
    }    
}

void cmd_rc(int argc, char **argv)
{
    rtc_command_t cmd;
    rtc_response_t response;
    
    cmd.type = RTC_CMD_READ_TIME;
    cmd.response_queue = xRtcResponseQueue;
    
    if (xQueueSend(xRtcQueue, &cmd, pdMS_TO_TICKS(100)) != pdTRUE) {
        printf("Error: Failed to send command\n");
        return;
    }
    
    if (xQueueReceive(xRtcResponseQueue, &response, pdMS_TO_TICKS(1000)) == pdTRUE) {
        printf("Clock: %02d:%02d:%02d\n", 
               response.time.hours, response.time.minutes, response.time.seconds);
    } else {
        printf("Error: Timeout\n");
    }
}


void cmd_sc(int argc, char **argv)
{
    if (argc != 4) { 
        printf("Usage: sc <hours> <minutes> <seconds>\n"); 
        return; 
    }
    
    rtc_command_t cmd;
    rtc_response_t response;
    
    // Read current date first
    cmd.type = RTC_CMD_READ_DATETIME;
    cmd.response_queue = xRtcResponseQueue;
    xQueueSend(xRtcQueue, &cmd, pdMS_TO_TICKS(100));
    xQueueReceive(xRtcResponseQueue, &response, pdMS_TO_TICKS(1000));
    
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
    
    // Read current time first
    cmd.type = RTC_CMD_READ_DATETIME;
    cmd.response_queue = xRtcResponseQueue;
    xQueueSend(xRtcQueue, &cmd, pdMS_TO_TICKS(100));
    xQueueReceive(xRtcResponseQueue, &response, pdMS_TO_TICKS(1000));
    
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