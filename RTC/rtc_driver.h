
#ifndef RTC_DRIVER_H
#define RTC_DRIVER_H

#include <stdint.h>
#include <time.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    uint8_t seconds;
    uint8_t minutes;
    uint8_t hours;
    uint8_t day;
    uint8_t month;
    uint16_t year;
} rtc_time_t;

// Initialize RTC with default time
void rtc_start(void);

// Set RTC time
void rtc_set_time(rtc_time_t* time);

// Get current RTC time
void rtc_get_time(rtc_time_t* time);

// Set time using standard time_t
void rtc_set_time_t(time_t seconds);

// Get time as time_t
time_t rtc_get_time_t(void);

double rtc_time_diff(rtc_time_t time1, rtc_time_t time2);

struct tm convert_rtc_time_to_tm(rtc_time_t time);

#ifdef __cplusplus
}
#endif

#endif // RTC_DRIVER_H
