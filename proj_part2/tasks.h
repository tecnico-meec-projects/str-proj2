
#ifndef TASKS_H
#define TASKS_H

#include <stdint.h>
#include "FreeRTOS.h"
#include "queue.h"
#include "semphr.h"
#include "task.h"
#include "rtc_driver.h"

#ifdef __cplusplus
extern "C" {
#endif


typedef enum {
    LCD_STATE_TIME,          // Show time only
    LCD_STATE_FULL,          // Show time + temperature + alarms
    LCD_STATE_BUBBLE_LEVEL,  // Show bubble level
    LCD_STATE_MESSAGE        // Show custom message
} lcd_display_state_t;


typedef enum {
    LCD_MSG_UPDATE_TIME,     // Update time display
    LCD_MSG_UPDATE_TEMP,     // Update temperature (future)
    LCD_MSG_UPDATE_ALARMS,   // Update alarm indicators (future)
    LCD_MSG_SET_STATE,       // Change display state
    LCD_MSG_SHOW_MESSAGE,    // Show custom message
    LCD_MSG_CLEAR            // Clear display
} lcd_message_type_t;

typedef struct {
    lcd_message_type_t type;
    lcd_display_state_t state;
    rtc_time_t time;
    float temperature;        // For future use
    uint8_t alarm_clock;      // For future use
    uint8_t alarm_temp;       // For future use
    char message[32];
} lcd_message_t;


typedef enum {
    RTC_CMD_READ_TIME,
    RTC_CMD_SET_TIME,
    RTC_CMD_READ_DATETIME,
    RTC_CMD_SET_DATE,
    RTC_CMD_SET_ALARM       // For future use
} rtc_command_type_t;

typedef struct {
    rtc_command_type_t type;
    rtc_time_t time;
    QueueHandle_t response_queue;
} rtc_command_t;

typedef struct {
    rtc_time_t time;
    uint8_t success;
} rtc_response_t;


extern QueueHandle_t xLcdQueue;
extern QueueHandle_t xRtcQueue;
extern SemaphoreHandle_t xRtcMutex;


void vRtcTask(void *pvParameters);
void vLcdTask(void *pvParameters);
void vConsoleTask(void *pvParameters);

#ifdef __cplusplus
}
#endif

#endif // TASKS_H