
#include <stdio.h>
#include <stdlib.h>
#include "diag/trace.h"
#include "stm32f4xx_hal.h"

#ifdef __cplusplus
extern "C" {
#endif

#include "ssd1306.h"
#ifdef __cplusplus
}
#endif

#define BSP_I2C_SPEED                          100000

static void I2Cx_Init(void);
static void I2Cx_MspInit(I2C_HandleTypeDef *hi2c);

I2C_HandleTypeDef I2cHandle;


int
main(int argc, char* argv[])
{
	// Initialize I2C2 - SDA=D14(PB11) SCL=D15(PB10)
	I2Cx_Init();
	trace_printf("I2C2 initialized.\n");

	ssd1306_Init(&I2cHandle);
	trace_printf("OLED initialized.\n");

	ssd1306_Fill(Black);
	ssd1306_SetCursor(0, 0);
	ssd1306_WriteString("------------------------", Font_7x10, White);
	ssd1306_SetCursor(0, SSD1306_HEIGHT-11);
	ssd1306_WriteString("------------------------", Font_7x10, White);



	uint8_t x = SSD1306_WIDTH/2-4-7;
	uint8_t y = SSD1306_HEIGHT/2-5;
	ssd1306_SetCursor(x, y);
	ssd1306_WriteString(" o ", Font_7x10, White);
	ssd1306_UpdateScreen(&I2cHandle);
	HAL_Delay(1000000);

  // Infinite loop
  while (1)
    {
	 x+=3;
	ssd1306_SetCursor(x, y);
	ssd1306_WriteString(" o ", Font_7x10, White);
	ssd1306_UpdateScreen(&I2cHandle);
	trace_printf("%d\n", x);

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
		I2Cx_MspInit(&I2cHandle);
		HAL_I2C_Init(&I2cHandle);
	}

}


static void I2Cx_MspInit(I2C_HandleTypeDef *hi2c) {
	GPIO_InitTypeDef gpio_init_structure;

	if (hi2c->Instance == I2C2) {
		/*** Configure the GPIOs ***/
		/* Enable GPIO clock */
		__HAL_RCC_GPIOB_CLK_ENABLE();

		/* Configure I2C2 SCL as alternate function */
		gpio_init_structure.Pin = GPIO_PIN_10;
		gpio_init_structure.Mode = GPIO_MODE_AF_OD;
		gpio_init_structure.Pull = GPIO_NOPULL;
		gpio_init_structure.Speed = GPIO_SPEED_FAST;
		gpio_init_structure.Alternate = GPIO_AF4_I2C2;
		HAL_GPIO_Init(GPIOB, &gpio_init_structure);

		/* Configure I2C SDA as alternate function */
		gpio_init_structure.Pin = GPIO_PIN_11;
		gpio_init_structure.Alternate = GPIO_AF4_I2C2;
		HAL_GPIO_Init(GPIOB, &gpio_init_structure);

		/*** Configure the I2C peripheral ***/
		/* Enable I2C clock */
		__HAL_RCC_I2C2_CLK_ENABLE();

		/* Force the I2C peripheral clock reset */
		__HAL_RCC_I2C2_FORCE_RESET();

		/* Release the I2C peripheral clock reset */
		__HAL_RCC_I2C2_RELEASE_RESET();

		/* Enable and set I2Cx Interrupt to a lower priority */
		HAL_NVIC_SetPriority(I2C2_EV_IRQn, 0x0F, 0x00);
		HAL_NVIC_EnableIRQ(I2C2_EV_IRQn);

		/* Enable and set I2Cx Interrupt to a lower priority */
		HAL_NVIC_SetPriority(I2C2_ER_IRQn, 0x0F, 0x00);
		HAL_NVIC_EnableIRQ(I2C2_ER_IRQn);
	}
}

// ----------------------------------------------------------------------------
