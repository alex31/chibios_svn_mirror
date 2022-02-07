/*
    ChibiOS - Copyright (C) 2006..2018 Giovanni Di Sirio

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at

        http://www.apache.org/licenses/LICENSE-2.0

    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.
*/

#include "ch.h"
#include "hal.h"

#include "chprintf.h"
#include "l3gd20.h"

/*===========================================================================*/
/* L3GD20 related.                                                           */
/*===========================================================================*/

/* L3GD20 Driver: This object represent an L3GD20 instance.*/
static L3GD20Driver L3GD20D1;

static int32_t gyroraw[L3GD20_GYRO_NUMBER_OF_AXES];
static float gyrocooked[L3GD20_GYRO_NUMBER_OF_AXES];

static char axisID[L3GD20_GYRO_NUMBER_OF_AXES] = {'X', 'Y', 'Z'};
static uint32_t i;

static const SPIConfig spicfg = {
  FALSE,
  NULL,
  GPIOD,
  GPIOD_GYRO_CS,
  SPI_CR1_BR | SPI_CR1_CPOL | SPI_CR1_CPHA,
  0
};

static L3GD20Config l3gd20cfg = {
  &SPID2,
  &spicfg,
  NULL,
  NULL,
  L3GD20_FS_250DPS,
  L3GD20_ODR_760HZ,
#if L3GD20_USE_ADVANCED
  L3GD20_BDU_CONTINUOUS,
  L3GD20_END_LITTLE,
  L3GD20_BW3,
  L3GD20_HPM_REFERENCE,
  L3GD20_HPCF_8,
  L3GD20_LP2M_ON,
#endif
};

/*===========================================================================*/
/* Generic code.                                                             */
/*===========================================================================*/

static BaseSequentialStream* chp = (BaseSequentialStream*)&SD2;
/*
 * Red LED blinker thread, times are in milliseconds.
 */
static THD_WORKING_AREA(waThread1, 128);
static THD_FUNCTION(Thread1, arg) {

  (void)arg;
  chRegSetThreadName("blinker");
  while (true) {
    palToggleLine(LINE_LED_RED);
    chThdSleepMilliseconds(250);
  }
}

/*
 * Application entry point.
 */
int main(void) {

  /*
   * System initializations.
   * - HAL initialization, this also initializes the configured device drivers
   *   and performs the board-specific initializations.
   * - Kernel initialization, the main() function becomes a thread and the
   *   RTOS is active.
   */
  halInit();
  chSysInit();

  /* Activates the serial driver 2 using the driver default configuration.*/
  sdStart(&SD2, NULL);

  /* Creates the blinker thread.*/
  chThdCreateStatic(waThread1, sizeof(waThread1), NORMALPRIO + 1, Thread1, NULL);

  /* L3GD20 Object Initialization.*/
  l3gd20ObjectInit(&L3GD20D1);

  /* Activates the L3GD20 driver.*/
  l3gd20Start(&L3GD20D1, &l3gd20cfg);

  /* Normal main() thread activity, printing MEMS data on the SD2.*/
  while (true) {
    l3gd20GyroscopeReadRaw(&L3GD20D1, gyroraw);
    chprintf(chp, "L3GD20 Gyroscope raw data...\r\n");
    for(i = 0; i < L3GD20_GYRO_NUMBER_OF_AXES; i++) {
      chprintf(chp, "%c-axis: %d\r\n", axisID[i], gyroraw[i]);
    }

    l3gd20GyroscopeReadCooked(&L3GD20D1, gyrocooked);
    chprintf(chp, "L3GD20 Gyroscope cooked data...\r\n");
    for(i = 0; i < L3GD20_GYRO_NUMBER_OF_AXES; i++) {
      chprintf(chp, "%c-axis: %.3f\r\n", axisID[i], gyrocooked[i]);
    }

    chThdSleepMilliseconds(100);
  }
  l3gd20Stop(&L3GD20D1);
}
