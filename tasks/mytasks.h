#ifndef TASKS_H
#define TASKS_H

#include <stdint.h>
#include <climits>
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"
#include "event_groups.h"
#include "rtc_driver.h"
#include "LM75B.h"
#include "imu_driver.h"
#include "config.h"


#define ALARM_TEMP          ((EventBits_t)(0x01 << 0))
#define ALARM_TIME          ((EventBits_t)(0x01 << 1)) 
#define ALARM_ACTIVE        (ALARM_TEMP | ALARM_TIME)

#define TEMP_EVT_PERIODIC   (1 << 0)
#define TEMP_EVT_CMD        (1 << 1)


typedef enum {
    LCD_MSG_UPDATE_TIME,
    LCD_MSG_UPDATE_TEMP,
    LCD_MSG_UPDATE_BUBBLE
} lcd_message_type_t;

typedef union {
    rtc_time_t time;
    float      temperature;
    tilt_t     tilt;
} lcd_message_payload_t;

typedef struct {
    lcd_message_type_t    type;
    lcd_message_payload_t data;
} lcd_message_t;


typedef enum {
    TEMP_CMD_READ_TEMP,
    TEMP_CMD_SET_TEMP_THRESHOLDS
} temp_message_type_t;

typedef struct {
    temp_message_type_t type;
    float               temp;
} temp_message_t;


typedef enum {
    RTC_CMD_SET_TIME,
    RTC_CMD_SET_DATE,
    RTC_CMD_READ_DATETIME,
    RTC_CMD_RESPONSE
} rtc_message_type_t;

typedef struct {
    rtc_message_type_t type;
    rtc_time_t         time;
} rtc_message_t;


extern QueueHandle_t      xLcdQueue;
extern QueueHandle_t      xRtcQueue;
extern QueueHandle_t      xConsoleQueue;

extern SemaphoreHandle_t  xRtcMutex;
extern SemaphoreHandle_t  xI2CMutex;

extern EventGroupHandle_t xAlarmEvents;

extern TimerHandle_t      xTempTimer;

extern TaskHandle_t       xTempTaskHandle;
extern TaskHandle_t       xRgbTaskHandle;
extern TaskHandle_t       xBubLvlTaskHandle;
extern TaskHandle_t       xHitBitTaskHandle;


#ifdef __cplusplus
extern "C" {
#endif

void vRtcTask(void *pvParameters);
void vLcdTask(void *pvParameters);
void vConsoleTask(void *pvParameters);
void vTempTask(void *pvParameters);
void vBubLvlTask(void *pvParameters);
void vAlarmTask(void *pvParameters);
void vRgbTask(void *pvParameters);
void vHitBitTask(void *pvParameters);

#ifdef __cplusplus
}
#endif

#endif // TASKS_H
