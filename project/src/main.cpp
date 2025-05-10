
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

#define _I2C_SPEED                          100000

#define BMA180_I2C_ADDR           			(0x40 << 1) 		//chip select to ground (7bit)

#define BMA180_REG_ACC_X_LSB   				0x02		//registers that hold the data
#define BMA180_REG_ACC_X_MSB   				0x03
#define BMA180_REG_ACC_Y_LSB   				0x04
#define BMA180_REG_ACC_Y_MSB   				0x05
#define BMA180_REG_ACC_Z_LSB   				0x06
#define BMA180_REG_ACC_Z_MSB   				0x07

#define BMA180_REG_GAIN_Z					0x34		// the 8th bit is wake_up bit (should be set to 1)

static void I2Cx_Init(void);
static void I2Cx_MspInit(I2C_HandleTypeDef *hi2c);

I2C_HandleTypeDef I2cHandle;


int
main(int argc, char* argv[])
{
	// Initialize I2C2 - SDA=D14(PB11) SCL=D15(PB10)

	I2Cx_Init();
	trace_printf("I2C2 initialized.\n");


	// Setting up accelerometer BMA180

	//	waking up:
	uint8_t value;
	HAL_I2C_Mem_Read(&I2cHandle, BMA180_I2C_ADDR, BMA180_REG_GAIN_Z, I2C_MEMADD_SIZE_8BIT, &value, 1, HAL_MAX_DELAY);
	trace_printf("Register 0x%02X = 0x%02X\n", BMA180_REG_GAIN_Z, value);
	value |= (1 << 7); 		//set the 8th bit to 1 (wake_up = True)
	HAL_I2C_Mem_Write(&I2cHandle, BMA180_I2C_ADDR, BMA180_REG_GAIN_Z, I2C_MEMADD_SIZE_8BIT, &value, 1, HAL_MAX_DELAY);

	// configuration parameters (maybe for later)

	//reading the data:
	int16_t acc_x = 0, acc_y = 0, acc_z = 0;
	uint8_t lsb = 0, msb = 0;
	//offset that i measured on a leveled surface(+-):
	int16_t x_offset = 10;
	int16_t y_offset = -290;
	int16_t z_offset = 4180;



	// Setting up OLED SSD1306
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


	HAL_I2C_Mem_Read(&I2cHandle, BMA180_I2C_ADDR, BMA180_REG_ACC_X_LSB, I2C_MEMADD_SIZE_8BIT, &lsb, 1, HAL_MAX_DELAY);
	HAL_I2C_Mem_Read(&I2cHandle, BMA180_I2C_ADDR, BMA180_REG_ACC_X_MSB, I2C_MEMADD_SIZE_8BIT, &msb, 1, HAL_MAX_DELAY);
	acc_x = ((int16_t)((msb << 8) | lsb)) >> 2;
	acc_x = acc_x - x_offset;
	HAL_I2C_Mem_Read(&I2cHandle, BMA180_I2C_ADDR, BMA180_REG_ACC_Y_LSB, I2C_MEMADD_SIZE_8BIT, &lsb, 1, HAL_MAX_DELAY);
	HAL_I2C_Mem_Read(&I2cHandle, BMA180_I2C_ADDR, BMA180_REG_ACC_Y_MSB, I2C_MEMADD_SIZE_8BIT, &msb, 1, HAL_MAX_DELAY);
	acc_y = ((int16_t)((msb << 8) | lsb)) >> 2;
	acc_y = acc_y - y_offset;
	HAL_I2C_Mem_Read(&I2cHandle, BMA180_I2C_ADDR, BMA180_REG_ACC_Z_LSB, I2C_MEMADD_SIZE_8BIT, &lsb, 1, HAL_MAX_DELAY);
	HAL_I2C_Mem_Read(&I2cHandle, BMA180_I2C_ADDR, BMA180_REG_ACC_Z_MSB, I2C_MEMADD_SIZE_8BIT, &msb, 1, HAL_MAX_DELAY);
	acc_z = ((int16_t)((msb << 8) | lsb)) >> 2;
	acc_z = acc_z - z_offset;

	trace_printf("X: %d, Y: %d, Z: %d\n", acc_x, acc_y, acc_z);

    }
}



static void I2Cx_Init(void) {
	if (HAL_I2C_GetState(&I2cHandle) == HAL_I2C_STATE_RESET) {
		I2cHandle.Instance = I2C2;
		I2cHandle.Init.ClockSpeed = _I2C_SPEED;
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
