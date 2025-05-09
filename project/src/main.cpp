
#include <stdio.h>
#include <stdlib.h>
#include "diag/trace.h"
#include "stm32f4xx_hal.h"

#define BSP_I2C_SPEED                          100000

static void I2Cx_Init(void);

I2C_HandleTypeDef I2cHandle;


int
main(int argc, char* argv[])
{
	// Initialize I2C2 - SDA=D14(PB11) SCL=D15(PB10)
	I2Cx_Init();

  // Infinite loop
  while (1)
    {
       // Add your code here.
    }
}


static void I2Cx_Init(void) {
	if (HAL_I2C_GetState(&I2cHandle) == HAL_I2C_STATE_RESET) {
		I2cHandle.Instance = I2C2;
		I2cHandle.Init.ClockSpeed = BSP_I2C_SPEED;
		I2cHandle.Init.DutyCycle = I2C_DUTYCYCLE_2;
		I2cHandle.Init.OwnAddress1 = 0;
		I2cHandle.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
		I2cHandle.Init.DualAddressMode = I2C_DUALADDRESS_DISABLED;
		I2cHandle.Init.OwnAddress2 = 0;
		I2cHandle.Init.GeneralCallMode = I2C_GENERALCALL_DISABLED;
		I2cHandle.Init.NoStretchMode = I2C_NOSTRETCH_DISABLED;

		/* Init the I2C */
		HAL_I2C_Init(&I2cHandle);
	}

}


// ----------------------------------------------------------------------------
