#include "imu_driver.h"

static MMA7660FC imu(p28, p27, 0x4C << 1);

void imu_init() {
    imu.init();
}

struct tilt_t imu_read() {
    struct tilt_t result;
    float placeholder;
    imu.read_Tilt(&result.x, &result.y, &placeholder);
    return result;
}
