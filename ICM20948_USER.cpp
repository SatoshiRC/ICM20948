/*
 * ICM20948_USER.cpp
 *
 *  Created on: Feb 10, 2023
 *      Author: OHYA Satoshi
 */

#include "ICM20948_USER.h"

void ICM20948_USER::init(){
	if(icm20948.whoami() == 0xea){
		HAL_GPIO_WritePin(GPIOA,GPIO_PIN_5,GPIO_PIN_SET);
	}

//		        printf("ICM20948 confirm\n");
		icm20948.reset();
		icm20948.pwrmgmt2(icm20948.DISABLE_SENSORS);
		icm20948.accelConfig(ICM20948::AccelSensitivity::SENS_2G,false,0);
		icm20948.gyroConfig(ICM20948::GyroSensitivity::SENS_500, false, 0);
		icm20948.pwrmgmt2(icm20948.ENABLE_SENSORS);
		icm20948.intPinConfig(0b01110000);
		icm20948.intenable();

		HAL_Delay(100);
//		        printf("initialized\n");

		icm20948.changeUserBank(2);
		uint8_t buffer2=0;
		//HAL_I2C_Mem_Read(&hi2c1, 0x68<<1,ICM20948::REGISTER.ACCEL_CONFIG,1,&buffer2,1,1000);
		icm20948.changeUserBank(0);

}

