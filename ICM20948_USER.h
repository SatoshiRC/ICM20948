/*
 * ICM20948_USER.h
 *
 *  Created on: Feb 10, 2023
 *      Author: OHYA Satoshi
 */

#ifndef ICM20948_ICM20948_USER_H_
#define ICM20948_ICM20948_USER_H_

#include "ICM20948.h"
#include "ICM20948_HAL.h"
#include "Madgwick/Madgwick.h"

class ICM20948_USER{
public:
	ICM20948_USER(I2C_HandleTypeDef *hi2c,ICM20948::Address address)
	:icm20948(hi2c, address){}

	void init();
	void update();

private:
	ICM20948_HAL icm20948;
};

#endif /* ICM20948_ICM20948_USER_H_ */
