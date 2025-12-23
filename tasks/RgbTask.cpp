#include "projdefs.h"
#include "mytasks.h"

static void temp_to_rgb(float temp, float tlow, float thigh,
                        PwmOut *r, PwmOut *g, PwmOut *b)
{
    float x;

    /* Clamp + normalize */
    if (temp <= tlow)
        x = 0.0f;
    else if (temp >= thigh)
        x = 1.0f;
    else
        x = (temp - tlow) / (thigh - tlow);

    /* Hue: blue (240°) → red (0°) */
    float hue = 240.0f * (1.0f - x);

    /* HSV → RGB (S=1, V=1) */
    float c = 1.0f;
    float h = hue / 60.0f;
    float xcol = c * (1.0f - fabsf(fmodf(h, 2.0f) - 1.0f));

    float r1 = 0, g1 = 0, b1 = 0;

    if      (h < 1) { r1 = c; g1 = xcol; }
    else if (h < 2) { r1 = xcol; g1 = c; }
    else if (h < 3) { g1 = c; b1 = xcol; }
    else if (h < 4) { g1 = xcol; b1 = c; }
    else if (h < 5) { r1 = xcol; b1 = c; }
    else            { r1 = c; b1 = xcol; }

    /* Common-anode inversion */
    *r = 1.0f - r1;
    *g = 1.0f - g1;
    *b = 1.0f - b1;
}

void vRgbTask(void *pvParameters)
{
    PwmOut          r(p23);
    PwmOut          g(p24);
    PwmOut          b(p25); 
    float           temp = 0;
    uint32_t raw;

    r.period_ms(1);
    g.period_ms(1);
    b.period_ms(1);

    for (;;) {

        xTaskNotifyWait(0, 0, &raw, portMAX_DELAY);
        memcpy(&temp, &raw, sizeof(float));

        temp_to_rgb(temp, (float)tlow, (float)thigh, &r, &g, &b);
        vTaskDelay(pdMS_TO_TICKS(50));
    }
}