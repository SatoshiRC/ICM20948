/*
 * ICM20948.h
 *
 *  Created on: Oct 3, 2022
 *      Author: zeroi
 */

#ifndef INC_ICM20948_H_
#define INC_ICM20948_H_

#include"i2c.h"
#include <array>

#define ICM20948_DISABLE_SENSORS 0x3F
#define ICM20948_ENABLE_SENSORS 0x00
#define ICM20948_BIT_H_RESET 0x80
#define ICM20948_BTT_CLK_PLL 0x01
#define ICM20948_BIT_INT_ACTL 0x80
#define ICM20948_BIT_INT_OPEN 0x40

class ICM20948 {
public:

	enum class AccelSensitivity: uint8_t{
		SENS_2G,
		SENS_4G,
		SENS_8G,
		SENS_16G,
	};

	enum class GyroSensitivity: uint8_t{
		SENS_250,
		SENS_500,
		SENS_1000,
		SENS_2000,
	};

	const struct{
		uint8_t WHO_AM_I = 0x00;
		uint8_t PWR_MGMT_1 = 0x06;
		uint8_t PWR_MGMT_2 = 0x07;
		uint8_t INT_PIN_CFG = 0x0F;
		uint8_t BANK_SEL = 0x7F;
		uint8_t ACCEL_CONFIG = 0x14;
		uint8_t GYRO_CONFIG = 0x01;
		uint8_t INT_ENABLE = 0x11;

	}REGISTER;

	enum class Address:uint8_t{
		LOW=0x68,
		HIGH=0x69,
	};
	ICM20948(I2C_HandleTypeDef *hi2c,Address address)
			:hi2c(hi2c),address(address){};



	uint8_t whoami();

	uint8_t pwrmgmt1(uint8_t data);
	uint8_t pwrmgmt2(uint8_t data);
	void reset();
	bool changeUserBank(const uint8_t bank);
	bool accelConfig(const AccelSensitivity fssel,const bool enableDLPF,const uint8_t configDLPF);
	bool gyroConfig(const GyroSensitivity fssel,const bool enableDLPF,const uint8_t configDLPF);
	void intPinConfig(uint8_t value);
	void intenable(uint8_t value=1);

	float getAccel(uint8_t axis);
	float getGyro(uint8_t axis);
	void getAccelBurst(std::array<float,3> &value);
	void getGyroBurst(std::array<float,3> &value);
	void get6ValueBurst(std::array<float,3> &accel, std::array<float,3> &gyro);

private:
	I2C_HandleTypeDef *hi2c;
	const Address address;
	AccelSensitivity _accelsensitivity;
	GyroSensitivity _gyrosensitivity;
	const float ACCEL_SENSITIVITY[4]={16384.0,8192.0,4096.0,2048.0};
	const float GYRO_SENSITIVITY[4]={131,65.5,32.8,16.4};


};
#endif /* INC_ICM20948_H_ */
