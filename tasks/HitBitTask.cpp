#include "mytasks.h"
#include "projdefs.h"
#include "mbed.h"


static DigitalOut led1(LED1);
static DigitalOut led2(LED2);
static DigitalOut led3(LED3);
static DigitalOut led4(LED4);


static InterruptIn center_button(p14);


static volatile uint8_t bit_pattern = 0x00;  // 4 bits representing LEDs
static volatile uint8_t rotation_pos = 0;    // Current rotation position (0-3)
static volatile bool button_pressed = false;


void button_isr(void)
{
    button_pressed = true;
}


static void update_leds(void)
{
    led1 = (bit_pattern >> 0) & 0x01;
    led2 = (bit_pattern >> 1) & 0x01;
    led3 = (bit_pattern >> 2) & 0x01;
    led4 = (bit_pattern >> 3) & 0x01;
}

// Rotate bits left
static void rotate_bits(void)
{
    uint8_t msb = (bit_pattern & 0x08) >> 3;  // Get leftmost bit
    bit_pattern = ((bit_pattern << 1) | msb) & 0x0F;  // Rotate and mask to 4 bits
}


static void win_animation(void)
{
    for (int i = 0; i < 3; i++) {
        led1 = led2 = led3 = led4 = 1;
        vTaskDelay(pdMS_TO_TICKS(200));
        led1 = led2 = led3 = led4 = 0;
        vTaskDelay(pdMS_TO_TICKS(200));
    }
}

void vHitBitTask(void *pvParameters)
{
    // Initialize game with one bit set
    bit_pattern = 0x01;  // Start with LED1 on
    rotation_pos = 0;
    
    // Attach interrupt to center button (falling edge = button press)
    center_button.fall(&button_isr);
    
    update_leds();
    
    TickType_t last_rotation = xTaskGetTickCount();
    const TickType_t rotation_period = pdMS_TO_TICKS(500);  // Rotate every 500ms
    
    for (;;) {
        if (!HB) {
            // Game disabled - turn off LEDs
            led1 = led2 = led3 = led4 = 0;
            vTaskDelay(pdMS_TO_TICKS(100));
            continue;
        }
        
        // Handle button press
        if (button_pressed) {
            button_pressed = false;
            
            // Check if LED4 (rightmost) is lit
            if (bit_pattern & 0x08) {
                
                bit_pattern &= ~0x08;
                
                // Check for win condition 
                if (bit_pattern == 0x00) {
                    win_animation();
                    // Reset game with one bit
                    bit_pattern = 0x01;
                }
            } else {

                for (int i = 0; i < 4; i++) {
                    if (!(bit_pattern & (1 << i))) {
                        bit_pattern |= (1 << i);
                        break;
                    }
                }
                // If all positions full, don't add more
            }
            
            update_leds();
            last_rotation = xTaskGetTickCount();  // Reset rotation timer
        }
        
        // Rotate bits periodically
        TickType_t now = xTaskGetTickCount();
        if ((now - last_rotation) >= rotation_period) {
            rotate_bits();
            update_leds();
            last_rotation = now;
        }
        
        vTaskDelay(pdMS_TO_TICKS(10));  
    }
}