// Copyright 2022 Google LLC
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include <cstdio>
#include <string>

#include "libs/base/gpio.h"
#include "libs/base/i2c.h"
#include "libs/base/led.h"
#include "libs/base/tasks.h"
#include "libs/base/console_m7.h"
#include "third_party/freertos_kernel/include/FreeRTOS.h"
#include "third_party/freertos_kernel/include/task.h"

#include "cli.h"
#include "libs/base/utils.h"
#include "libs/base/check.h"

extern coralmicro::I2cConfig config;

extern "C" void app_main(void* param) {
  (void)param;
  auto user_led = coralmicro::Led::kUser;


  // Turn on Status LED to show the board is on.
  coralmicro::LedSet(coralmicro::Led::kStatus, true);

  constexpr coralmicro::Gpio kGpiosToTest[] = {
      coralmicro::Gpio::kAA,    coralmicro::Gpio::kAB,
      coralmicro::Gpio::kBtHostWake,      coralmicro::Gpio::kBtDevWake
  };

  for (auto gpio : kGpiosToTest) {
    coralmicro::GpioSetMode(gpio, coralmicro::GpioMode::kOutput);
  }

  xTaskCreate(&vCommandConsoleTask, "xCommandCLI", configMINIMAL_STACK_SIZE,
              &user_led, coralmicro::kAppTaskPriority, nullptr);


  vTaskSuspend(nullptr);
}

/*
[[noreturn]] void blink_task(void* param) {
  auto led_type = static_cast<coralmicro::Led*>(param);
  bool on = true;
  while (true) {
    on = !on;
    coralmicro::LedSet(*led_type, on);
    vTaskDelay(pdMS_TO_TICKS(500));
  }
}

extern "C" [[noreturn]] void app_main(void* param) {
  (void)param;

  printf("\033[2J");
  printf("\r\rWelcome to Coral CLI version 0.1\r\n");
  // Turn on Status LED to show the board is on.
  LedSet(coralmicro::Led::kStatus, true);

  printf("System ready.\r\n\r\n>>");

  char ch;
  char val[256];
  int i = 0;
  coralmicro::GpioInit();

  while (true) {
    int bytes = coralmicro::ConsoleM7::GetSingleton()->Read(&ch, 1);
    if (bytes == 1) {
      coralmicro::ConsoleM7::GetSingleton()->Write(&ch, 1);
      if (ch == 13) {
        coralmicro::ConsoleM7::GetSingleton()->Write("\33[2K\r>", 6);
        i = 0;

        if (strcmp(val, "help") == 0)
          coralmicro::ConsoleM7::GetSingleton()->Write("help youself!\r\n>", 15);
        else if (strstr(val, "userled on") != 0){
          int reg = 0;
          sscanf(val+9, "%d", &reg);
          coralmicro::GpioSet(coralmicro::Gpio::kUserLed, true);
          coralmicro::ConsoleM7::GetSingleton()->Write("ack userled on\r\n>", 16);
        }
        else if (strstr(val, "userled off") != 0){
          int reg = 0;
          sscanf(val+9, "%d", &reg);
          coralmicro::GpioSet(coralmicro::Gpio::kUserLed, false);
          coralmicro::ConsoleM7::GetSingleton()->Write("ack userled off\r\n>", 17);
        }
        else if (strstr(val, "statusled on") != 0){
          int reg = 0;
          sscanf(val+9, "%d", &reg);
          coralmicro::GpioSet(coralmicro::Gpio::kStatusLed, true);
          coralmicro::ConsoleM7::GetSingleton()->Write("ack userled on\r\n>", 16);
        }
        else if (strstr(val, "statusled off") != 0){
          int reg = 0;
          sscanf(val+9, "%d", &reg);
          coralmicro::GpioSet(coralmicro::Gpio::kStatusLed, false);
          coralmicro::ConsoleM7::GetSingleton()->Write("ack userled off\r\n>", 17);
        }
        else
          coralmicro::ConsoleM7::GetSingleton()->Write("unknown command\r\n>", 17);

      }
      else {
        val[i++] = ch;
        val[i] = 0;
      }
    }
    taskYIELD();
  }
}
*/
/*
  while (true) {
    int bytes = coralmicro::ConsoleM7::GetSingleton()->Read(&ch, 1);
    if (bytes == 1) {
      coralmicro::ConsoleM7::GetSingleton()->Write(&ch, 1);
      if (ch == 13) {
        coralmicro::ConsoleM7::GetSingleton()->Write("\33[2K\r", 5);
        i = 0;

        if (strcmp(val, "hello") == 0)
          coralmicro::ConsoleM7::GetSingleton()->Write("hi\r\n>", 6);

      }

    }
  }

*/
