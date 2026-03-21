#include "freertos/timers.h"

// Timers handles
extern TimerHandle_t xUplinkTimerHandle;

// Timers callbacks prototypes
void vUplinkTimerCallback(TimerHandle_t xTimer);