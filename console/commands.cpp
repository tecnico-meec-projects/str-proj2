#include "mytasks.h"
#include "records.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include "projdefs.h"
#include "rtc_driver.h"
#include "config.h"


static bool validate_int_arg(const char* arg, int* result, int min, int max, const char* name)
{
    char* endptr;
    long val = strtol(arg, &endptr, 10);
    

    if (*endptr != '\0' || endptr == arg) {
        printf("Error: '%s' is not a valid integer for %s\r\n", arg, name);
        return false;
    }
    

    if (val < min || val > max) {
        printf("Error: %s must be between %d and %d (got %ld)\r\n", name, min, max, val);
        return false;
    }
    
    *result = (int)val;
    return true;
}

void cmd_test (int argc, char** argv)
{
    int i;


    for (i=0; i<argc; i++)
        printf("\r\nargv[%d] = %s", i, argv[i]);
}

void cmd_rdt(int argc, char **argv)
{
    rtc_message_t   cmd;
    rtc_message_t   response;
    
    cmd.type = RTC_CMD_READ_DATETIME;

    if (xQueueSend(xRtcQueue, &cmd, pdMS_TO_TICKS(100)) != pdTRUE) {
        printf("Error: Failed to send command\r\n");
        return;
    }
    
    if (xQueueReceive(xConsoleQueue, &response, pdMS_TO_TICKS(1000)) == pdTRUE) {
        printf("Date/Time: %02d/%02d/%04d %02d:%02d:%02d\r\n",
               response.time.day, response.time.month, response.time.year,
               response.time.hours, response.time.minutes, response.time.seconds);
    } else {
        printf("Error: Timeout\r\n");
    }    
}

void cmd_rc(int argc, char **argv)
{
    rtc_message_t   cmd;
    rtc_message_t   response;
    
    cmd.type = RTC_CMD_READ_DATETIME;
    
    if (xQueueSend(xRtcQueue, &cmd, pdMS_TO_TICKS(100)) != pdTRUE) {
        printf("Error: Failed to send command\r\n");
        return;
    }
    
    if (xQueueReceive(xConsoleQueue, &response, pdMS_TO_TICKS(1000)) == pdTRUE) {
        printf("Clock: %02d:%02d:%02d\r\n", 
               response.time.hours, response.time.minutes, response.time.seconds);
    } else {
        printf("Error: Timeout\r\n");
    }
}


void cmd_sc(int argc, char **argv)
{
    if (argc != 4) { 
        printf("Usage: sc <hours> <minutes> <seconds>\r\n"); 
        return; 
    }
    
    rtc_message_t   cmd;
    int h, m, s;
    
    // Validate arguments
    if (!validate_int_arg(argv[1], &h, 0, 23, "hours")) return;
    if (!validate_int_arg(argv[2], &m, 0, 59, "minutes")) return;
    if (!validate_int_arg(argv[3], &s, 0, 59, "seconds")) return;
    
    // Set new time
    cmd.type = RTC_CMD_SET_TIME;
    cmd.time.hours = h;
    cmd.time.minutes = m;
    cmd.time.seconds = s;
    
    if (xQueueSend(xRtcQueue, &cmd, pdMS_TO_TICKS(100)) == pdTRUE) {
        printf("Clock set to %02d:%02d:%02d\r\n", 
               cmd.time.hours, cmd.time.minutes, cmd.time.seconds);
    } else {
        printf("Error: Failed to set clock\r\n");
    }
}


void cmd_sd(int argc, char **argv)
{
    if (argc != 4) { 
        printf("Usage: sd <day> <month> <year>\r\n");
        return; 
    }
    
    rtc_message_t   cmd;
    int d, m, y;
    
    // Validate arguments
    if (!validate_int_arg(argv[1], &d, 1, 31, "day")) return;
    if (!validate_int_arg(argv[2], &m, 1, 12, "month")) return;
    if (!validate_int_arg(argv[3], &y, 1970, 2037, "year")) return;
    
    // Set new date
    cmd.type = RTC_CMD_SET_DATE;
    cmd.time.day = d;
    cmd.time.month = m;
    cmd.time.year = y;
    
    if (xQueueSend(xRtcQueue, &cmd, pdMS_TO_TICKS(100)) == pdTRUE) {
        printf("Date set to %02d/%02d/%04d\r\n", 
               cmd.time.day, cmd.time.month, cmd.time.year);
    } else {
        printf("Error: Failed to set date\r\n");
    }
}

void cmd_rt(int argc, char **argv)
{
    temp_message_t  response;

    xTaskNotify(xTempTaskHandle, TEMP_EVT_CMD, eSetBits);

    if (xQueueReceive(xConsoleQueue, &response, pdMS_TO_TICKS(1000)) == pdTRUE) {
        printf("T=%.1fC\r\n", response.temp);
    } else {
        printf("Error: Timeout\r\n");
    }
}

void cmd_rmm(int argc, char **argv)
{
    printf("Minimum temperature: %.1fC, maximum temperature: %.1fC\r\n", min_temp, max_temp);
}

void cmd_cmm(int argc, char **)
{
    max_temp = MAX_TEMP_DEFAULT;
    min_temp = MIN_TEMP_DEFAULT;
    printf("Max and min records reset\r\n");
}

void cmd_rp(int argc, char **argv)
{
    printf("pmon=%d\r\n", pmon);
    printf("tala=%d\r\n", tala);
}

void cmd_mmp(int argc, char **argv)
{
    if (argc != 2) { 
        printf("Usage: mmp <period>\r\n");
        return; 
    }

    int p;
    if (!validate_int_arg(argv[1], &p, 0, 99, "monitoring period")) return;

    uint8_t prev_pmon = pmon;
    pmon = p;
    
    if (pmon == 0) {
        xTimerStop(xTempTimer, 0);
        printf("Monitoring period disabled (was %d seconds)\r\n", prev_pmon);
    } else {
        if (prev_pmon == 0)
            xTimerStart(xTempTimer, 0);
        xTimerChangePeriod(xTempTimer, pdMS_TO_TICKS(pmon * 1000), pdMS_TO_TICKS(1000));
        printf("Monitoring period changed from %d to %d seconds\r\n", prev_pmon, pmon);
    }
}

void cmd_mta(int argc, char **argv)
{
    if (argc != 2) {
        printf("Usage: mta <time>\r\n");
        return;
    }
    
    int t;
    if (!validate_int_arg(argv[1], &t, 0, 60, "alarm time")) return;
    
    tala = t;
    printf("Alarm duration set to %d seconds\r\n", tala);
}

void cmd_rai(int argc, char **argv)
{
    printf("Alarm clock set to %02d:%02d:%02d\r\n", ALAH, ALAM, ALAS);
    printf("Temperature thresholds: tlow=%d°C, thigh=%d°C\r\n", tlow, thigh);
    printf("Clock alarm: %s\r\n", ALAC ? "enabled" : "disabled");
    printf("Temperature alarm: %s\r\n", ALAT ? "enabled" : "disabled");
}

void cmd_sac(int argc, char **argv)
{
    if (argc != 4) {
        printf("Usage: sac <hours> <minutes> <seconds>\r\n");
        return;
    }

    int h, m, s;
    if (!validate_int_arg(argv[1], &h, 0, 23, "hours")) return;
    if (!validate_int_arg(argv[2], &m, 0, 59, "minutes")) return;
    if (!validate_int_arg(argv[3], &s, 0, 59, "seconds")) return;

    ALAH = h;
    ALAM = m;
    ALAS = s;

    printf("Clock alarm set to %02d:%02d:%02d\r\n", ALAH, ALAM, ALAS);
}

void cmd_sat(int argc, char **argv)
{
    if (argc != 3) {
        printf("Usage: sat <tlow> <thigh>\r\n");
        return;
    }

    int tl, th;
    if (!validate_int_arg(argv[1], &tl, 0, 50, "tlow")) return;
    if (!validate_int_arg(argv[2], &th, 0, 50, "thigh")) return;
    
    if (tl >= th) {
        printf("Error: tlow must be less than thigh\r\n");
        return;
    }

    tlow = tl;
    thigh = th;

    printf("Temperature thresholds set: tlow=%d°C, thigh=%d°C\r\n", tlow, thigh);
}

void cmd_adac(int argc, char **argv)
{
    if (argc != 2) {
        printf("Usage: adac <1|0>\r\n");
        return;
    }

    if (strcmp(argv[1], "1") == 0) {
        ALAC = true;
        printf("Clock alarm enabled\r\n");
    } else if (strcmp(argv[1], "0") == 0) {
        ALAC = false;
        xEventGroupClearBits(xAlarmEvents, ALARM_TIME);
        printf("Clock alarm disabled\r\n");
    } else {
        printf("Error: argument must be 0 or 1\r\n");
    }
}

void cmd_adat(int argc, char **argv)
{
    if (argc != 2) {
        printf("Usage: adat <1|0>\r\n");
        return;
    }

    if (strcmp(argv[1], "1") == 0) {
        ALAT = true;
        printf("Temperature alarm enabled\r\n");
    } else if (strcmp(argv[1], "0") == 0) {
        ALAT = false;
        xEventGroupClearBits(xAlarmEvents, ALARM_TEMP);
        printf("Temperature alarm disabled\r\n");
    } else {
        printf("Error: argument must be 0 or 1\r\n");
    }
}

void cmd_rts(int argc, char **argv)
{
    printf("Bubble Level: %s\r\n", BL ? "enabled" : "disabled");
    printf("Hit Bit: %s\r\n", HB ? "enabled" : "disabled");
    printf("Config Sound: %s\r\n", CS ? "enabled" : "disabled");
}

void cmd_adbl(int argc, char **argv)
{
    if (argc != 2) {
        printf("Usage: adbl <1|0>\r\n");
        return;
    }

    if (strcmp(argv[1], "1") == 0) {
        BL = true;
        printf("Bubble Level enabled\r\n");
    } else if (strcmp(argv[1], "0") == 0) {
        BL = false;
        printf("Bubble Level disabled\r\n");
    } else {
        printf("Error: argument must be 0 or 1\r\n");
    }
}

void cmd_adhb(int argc, char **argv)
{
    if (argc != 2) {
        printf("Usage: adhb <1|0>\r\n");
        return;
    }

    if (strcmp(argv[1], "1") == 0) {
        HB = true;
        printf("Hit Bit enabled\r\n");
    } else if (strcmp(argv[1], "0") == 0) {
        HB = false;
        printf("Hit Bit disabled\r\n");
    } else {
        printf("Error: argument must be 0 or 1\r\n");
    }
}

void cmd_adcs(int argc, char **argv)
{
    if (argc != 2) {
        printf("Usage: adcs <1|0>\r\n");
        return;
    }

    if (strcmp(argv[1], "1") == 0) {
        CS = true;
        printf("Config Sound enabled\r\n");
    } else if (strcmp(argv[1], "0") == 0) {
        CS = false;
        printf("Config Sound disabled\r\n");
    } else {
        printf("Error: argument must be 0 or 1\r\n");
    }
}