
#include "lcd_driver.h"

// Create LCD object 
// C12832(PinName mosi, PinName sck, PinName reset, PinName a0, PinName cs)
static C12832 lcd(p5, p7, p6, p8, p11);


void lcd_init(void)
{
    lcd.cls();
}

void lcd_clear(void)
{
    lcd.cls();
}


void lcd_print(uint8_t x, uint8_t y, const char* str)
{
    lcd.locate(x, y);
    lcd.printf("%s", str);
}


void lcd_update(void)
{
    // C12832 library updates automatically, so this is a no-op
    // Kept for API compatibility
}

void lcd_line(int x0, int y0, int x1, int y1)
{
    lcd.line(x0, y0, x1, y1, 1);
}