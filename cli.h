#include "libs/base/gpio.h"
#include "libs/base/led.h"
#include "libs/base/tasks.h"
#include "libs/base/console_m7.h"
#include "third_party/freertos_kernel/include/FreeRTOS.h"
#include "third_party/freertos_kernel/include/task.h"

[[noreturn]] void vCommandConsoleTask( void *pvParameters );



