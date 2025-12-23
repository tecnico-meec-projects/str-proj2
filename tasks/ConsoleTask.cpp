#include "mytasks.h"

extern void monitor(Serial *pc);

void vConsoleTask(void *pvParameters)
{
    Serial pc(USBTX, USBRX);

    pc.baud(115200);

    for (;;) {
        monitor(&pc);
    }
}