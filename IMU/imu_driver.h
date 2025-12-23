#ifndef IMU_DRIVER_H
#define IMU_DRIVER_H

#include "MMA7660FC/MMA7660FC.h"

#ifdef __cplusplus
extern "C" {
#endif

struct tilt_t {
    float x;
    float y;
};

void imu_init();
struct tilt_t imu_read();

#ifdef __cplusplus
}
#endif

#endif // IMU_DRIVER_H
