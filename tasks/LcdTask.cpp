
#include "tasks.h"
#include "lcd_driver.h"
#include "projdefs.h"
#include <stdio.h>
#include <string.h>

#define LCD_WIDTH   128
#define LCD_HEIGHT   32

#define BUBBLE_RADIUS  3

#define BUBBLE_AREA_X0  65
#define BUBBLE_AREA_X1  127
#define BUBBLE_AREA_Y0  0
#define BUBBLE_AREA_Y1  31
#define MAX_ANGLE       20

static void draw_bubble(tilt_t tilt)
{
    int center_x = (BUBBLE_AREA_X0 + BUBBLE_AREA_X1) / 2;
    int center_y = (BUBBLE_AREA_Y0 + BUBBLE_AREA_Y1) / 2;

    int len_x = BUBBLE_AREA_X1 - BUBBLE_AREA_X0 - 2 * BUBBLE_RADIUS;
    int len_y = BUBBLE_AREA_Y1 - BUBBLE_AREA_Y0 - 2 * BUBBLE_RADIUS;

    int dx = -(tilt.x * len_x) / (2 * MAX_ANGLE);
    int dy = (tilt.y * len_y) / (2 * MAX_ANGLE);

    int bubble_x = center_x + dx;
    int bubble_y = center_y + dy;

    /* Clamp */
    if (bubble_x < BUBBLE_AREA_X0 + BUBBLE_RADIUS)
        bubble_x = BUBBLE_AREA_X0 + BUBBLE_RADIUS;
    if (bubble_x > BUBBLE_AREA_X1 - BUBBLE_RADIUS)
        bubble_x = BUBBLE_AREA_X1 - BUBBLE_RADIUS;

    if (bubble_y < BUBBLE_AREA_Y0 + BUBBLE_RADIUS)
        bubble_y = BUBBLE_AREA_Y0 + BUBBLE_RADIUS;
    if (bubble_y > BUBBLE_AREA_Y1 - BUBBLE_RADIUS)
        bubble_y = BUBBLE_AREA_Y1 - BUBBLE_RADIUS;


    // clear this section of screen
    lcd_clear_area(BUBBLE_AREA_X0, BUBBLE_AREA_Y0, BUBBLE_AREA_X1, BUBBLE_AREA_Y1);
    // draw center cross hair
    lcd_line(center_x - BUBBLE_RADIUS, center_y, center_x + BUBBLE_RADIUS, center_y);   // horizontal
    lcd_line(center_x, center_y - BUBBLE_RADIUS, center_x, center_y + BUBBLE_RADIUS);   // vertical
    /* Draw bubble */
    lcd_circle(bubble_x, bubble_y, BUBBLE_RADIUS);
}

void vLcdTask(void *pvParameters)
{
    lcd_message_t msg;
    lcd_display_state_t current_state = LCD_STATE_FULL;

    // Keep track of last known values
    rtc_time_t  current_time = {0};
    float       current_temp = 0.0f;
    uint8_t     alarm_clock = 0, alarm_temp = 0;
    tilt_t      tilt = {0};

    bool        time_dirty = false;
    bool        temp_dirty = false;
    bool        alarms_dirty = false;
    bool        state_dirty = false;
    bool        tilt_dirty = false;

    char line1[16], line2[16], line3[16];

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

                case LCD_MSG_UPDATE_BUBBLE:
                    tilt = msg.tilt;
                    tilt_dirty = true;
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
                        lcd_line(64, 0, 64, 31);
                        state_dirty = false;
                        time_dirty = temp_dirty = alarms_dirty = tilt_dirty = true;
                    }
                    if (time_dirty) {
                        sprintf(line1, "%02d:%02d:%02d",
                                current_time.hours,
                                current_time.minutes,
                                current_time.seconds);
                        lcd_print(0, 0, line1);
                        lcd_line(64, 0, 64, 31);
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
                        lcd_print(0, 20, line3);
                        temp_dirty = false;
                    }
                    if (tilt_dirty) {
                        draw_bubble(tilt);
                        tilt_dirty = false;
                    }
                    lcd_update();
                    break;
                default:
                    break;
            }
        }
    }
}
