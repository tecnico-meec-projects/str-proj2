
#ifndef LCD_DRIVER_H
#define LCD_DRIVER_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

void lcd_init(void);
void lcd_clear(void);
void lcd_print(uint8_t x, uint8_t y, const char* str);
void lcd_update(void);

#ifdef __cplusplus
}
#endif

#endif // LCD_DRIVER_H