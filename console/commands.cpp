#include "mytasks.h"
#include "records.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include "projdefs.h"
#include "rtc_driver.h"
#include "config.h"

void cmd_test (int argc, char** argv)
{
    int i;

    /* exemplo -- escreve argumentos */
    for (i=0; i<argc; i++)
        printf ("\nargv[%d] = %s", i, argv[i]);
}

void cmd_rdt(int argc, char **argv)
{
    rtc_message_t   cmd;
    rtc_message_t   response;
    
    cmd.type = RTC_CMD_READ_DATETIME;

    if (xQueueSend(xRtcQueue, &cmd, pdMS_TO_TICKS(100)) != pdTRUE) {
        printf("Error: Failed to send command\n");
        return;
    }
    
    if (xQueueReceive(xConsoleQueue, &response, pdMS_TO_TICKS(1000)) == pdTRUE) {
        printf("Date/Time: %02d/%02d/%04d %02d:%02d:%02d\n",
               response.time.day, response.time.month, response.time.year,
               response.time.hours, response.time.minutes, response.time.seconds);
    } else {
        printf("Error: Timeout\n");
    }    
}

void cmd_rc(int argc, char **argv)
{
    rtc_message_t   cmd;
    rtc_message_t   response;
    
    cmd.type = RTC_CMD_READ_DATETIME;
    
    if (xQueueSend(xRtcQueue, &cmd, pdMS_TO_TICKS(100)) != pdTRUE) {
        printf("Error: Failed to send command\n");
        return;
    }
    
    if (xQueueReceive(xConsoleQueue, &response, pdMS_TO_TICKS(1000)) == pdTRUE) {
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
    
    rtc_message_t   cmd;
    rtc_message_t   response;
    
    
    // Set new time
    cmd.type = RTC_CMD_SET_TIME;
    cmd.time.hours = atoi(argv[1]);
    cmd.time.minutes = atoi(argv[2]);
    cmd.time.seconds = atoi(argv[3]);
    
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
    
    rtc_message_t   cmd;
    rtc_message_t   response;
    
    // Set new date
    cmd.type = RTC_CMD_SET_DATE;
    cmd.time.day = atoi(argv[1]);
    cmd.time.month = atoi(argv[2]);
    cmd.time.year = atoi(argv[3]);
    
    if (xQueueSend(xRtcQueue, &cmd, pdMS_TO_TICKS(100)) == pdTRUE) {
        printf("Date set to %02d/%02d/%04d\n", 
               cmd.time.day, cmd.time.month, cmd.time.year);
    } else {
        printf("Error: Failed to set date\n");
    }
}

void cmd_rt(int argc, char **argv)
{
    temp_message_t  response;

    xTaskNotify(xTempTaskHandle, TEMP_EVT_CMD, eSetBits);

    if (xQueueReceive(xConsoleQueue, &response, pdMS_TO_TICKS(1000)) == pdTRUE) {
        printf("T=%.1fC\n", response.temp);
    } else {
        printf("Error: Timeout\n");
    }

}

void cmd_rmm(int argc, char **argv)
{
    printf("Minimum temperature: %.1fC, maximum temperature: %.1fC\n", min_temp, max_temp);
}

void cmd_cmm(int argc, char **)
{
    max_temp = MAX_TEMP_DEFAULT;
    min_temp = MIN_TEMP_DEFAULT;
    printf("Max and min records reset\n");
}

void cmd_rp(int argc, char **argv)
{
    printf("tmon=%d\n", pmon);
    printf("tala=%d\n", tala);
}

void cmd_mmp(int argc, char **argv)
{
    if (argc != 2) { 
        printf("Usage: mmp <pmon>\n");
        return; 
    }

    uint8_t prev_pmon = pmon;
    printf("pmon successfully changed from %d seconds to ", prev_pmon);
    pmon = atoi(argv[1]);
    if (pmon == 0)
        xTimerStop(xTempTimer, 0);
    else
    {
        if (prev_pmon == 0)
            xTimerStart(xTempTimer, 0);
        xTimerChangePeriod(xTempTimer, pdMS_TO_TICKS(pmon * 1000), pdMS_TO_TICKS(1000));
    }
    printf("to %d seconds\n", pmon);
}

void cmd_rai(int argc, char **argv)
{
    printf("alarm clock set to %02d:%02d:%02d,\n tlow = %d, thigh = %d,\n clock alarm %s, temperature alarm %s\n",
            ALAH, ALAM, ALAS,
            tlow, thigh,
            ALAC ? "enabled" : "disabled",
            ALAT  ? "enabled" : "disabled");
}

static uint8_t clamp(int v, int min, int max)
{
    if (v < min) return min;
    if (v > max) return max;
    return (uint8_t)v;
}

void cmd_sac(int argc, char **argv)
{
    if (argc != 4) {
        printf("Usage: sac <h> <m> <s>\n");
        return;
    }

    ALAH = clamp(atoi(argv[1]), 0, 23);
    ALAM = clamp(atoi(argv[2]), 0, 59);
    ALAS = clamp(atoi(argv[3]), 0, 59);

    printf("Clock alarm set to %02d:%02d:%02d\n", 
            ALAH, ALAM, ALAS);
}

void cmd_sat(int argc, char **argv)
{
    if (argc != 3) {
        printf("Usage: sat <tl> <th>\n");
        return;
    }

    tlow = clamp(atoi(argv[1]), 0, 50);
    thigh = clamp(atoi(argv[2]), 0, 50);

    printf("tlow set to %dC, thigh set to %dC\n", 
            tlow, thigh);
}

void cmd_adac(int argc, char **argv)
{
    if (argc != 2) {
        printf("Usage: adac <1|0>\n");
        return;
    }

    if (strcmp(argv[1], "1") == 0) {
        ALAC = true;
    } else if (strcmp(argv[1], "0") == 0) {
        ALAC = false;
    } else {
        printf("Error: argument must be 0 or 1\n");
        return;
    }

    printf("Clock alarm %s\n", ALAC ? "enabled" : "disabled");
}

void cmd_adat(int argc, char **argv)
{
    if (argc != 2) {
        printf("Usage: adat <1|0>\n");
        return;
    }

    if (strcmp(argv[1], "1") == 0) {
        ALAT = true;
    } else if (strcmp(argv[1], "0") == 0) {
        ALAT = false;
    } else {
        printf("Error: argument must be 0 or 1\n");
        return;
    }

    printf("Temperature alarm %s\n", ALAT ? "enabled" : "disabled");
}

void cmd_rts(int argc, char **argv)
{
    printf("Bubble Level: %s, Hit Bit: %s, Config Sound: %s\n",
            BL ? "enabled" : "disabled",
            HB ? "enabled" : "disabled",
            CS ? "enabled" : "disabled");
}

void cmd_adbl(int argc, char **argv)
{
    if (argc != 2) {
        printf("Usage: adbl <1|0>\n");
        return;
    }

    bool old_BL = BL;

    if (strcmp(argv[1], "1") == 0) {
        BL = true;
    } else if (strcmp(argv[1], "0") == 0) {
        BL = false;
    } else {
        printf("Error: argument must be 0 or 1\n");
        return;
    }

    printf("Bubble Level %s\n", BL ? "enabled" : "disabled");
}

void cmd_adhb(int argc, char **argv)
{
    if (argc != 2) {
        printf("Usage: adhb <1|0>\n");
        return;
    }

    if (strcmp(argv[1], "1") == 0) {
        HB = true;
    } else if (strcmp(argv[1], "0") == 0) {
        HB = false;
    } else {
        printf("Error: argument must be 0 or 1\n");
        return;
    }

    printf("Hit Bit %s\n", HB ? "enabled" : "disabled");

    // TODO: actually enable/disable it
}

void cmd_adcs(int argc, char **argv)
{
    if (argc != 2) {
        printf("Usage: adcs <1|0>\n");
        return;
    }

    if (strcmp(argv[1], "1") == 0) {
        CS = true;
    } else if (strcmp(argv[1], "0") == 0) {
        CS = false;
    } else {
        printf("Error: argument must be 0 or 1\n");
        return;
    }

    printf("Config Sound %s\n", CS ? "enabled" : "disabled");

    //TODO: actually enable/disable
}