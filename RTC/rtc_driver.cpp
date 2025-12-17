
#include "rtc_driver.h"
#include "mbed.h"
#include <time.h>


void rtc_start(void)
{
    // Set default time: 12:00:00 13/12/2025
    struct tm t;
    t.tm_sec = 0;           // 0-59
    t.tm_min = 0;           // 0-59
    t.tm_hour = 12;         // 0-23
    t.tm_mday = 13;         // 1-31
    t.tm_mon = 11;          // 0-11 (December = 11)
    t.tm_year = 125;        // years since 1900 (2025 = 125)
    
    // Convert to time_t and set RTC
    time_t seconds = mktime(&t);
    set_time(seconds);      // mbed function to set RTC
}


void rtc_set_time(rtc_time_t* time)
{
    struct tm t;
    t.tm_sec = time->seconds;
    t.tm_min = time->minutes;
    t.tm_hour = time->hours;
    t.tm_mday = time->day;
    t.tm_mon = time->month - 1;        // tm_mon is 0-11
    t.tm_year = time->year - 1900;     // tm_year is years since 1900
    
    time_t seconds = mktime(&t);
    set_time(seconds);                  // mbed function
}


void rtc_get_time(rtc_time_t* time)
{
    time_t seconds = ::time(NULL);      // mbed function to read RTC
    struct tm *t = localtime(&seconds);
    
    time->seconds = t->tm_sec;
    time->minutes = t->tm_min;
    time->hours = t->tm_hour;
    time->day = t->tm_mday;
    time->month = t->tm_mon + 1;        // Convert back to 1-12
    time->year = t->tm_year + 1900;     // Convert back to actual year
}


void rtc_set_time_t(time_t seconds)
{
    set_time(seconds);
}


time_t rtc_get_time_t(void)
{
    return ::time(NULL);
}