
#ifndef LCD_DRIVER_H
#define LCD_DRIVER_H

#include <stdint.h>
#include "C12832/C12832.h"

#ifdef __cplusplus
extern "C" {
#endif

void lcd_init(void);
void lcd_clear(void);
void lcd_print(uint8_t x, uint8_t y, const char* str);
void lcd_line(int x0, int y0, int x1, int y1);
void lcd_circle(int x, int y, int r);
void lcd_clear_area(int x0, int y0, int x1, int y1);
void lcd_update(void);

#ifdef __cplusplus
}
#endif

#endif // LCD_DRIVER_H
