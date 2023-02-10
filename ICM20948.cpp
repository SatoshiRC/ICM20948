/*
 * ICM20948.cpp
 *
 *  Created on: Oct 3, 2022
 *      Author: zeroi
 */

#include <ICM20948.h>

//ICM20948::ICM20948() {
//	// TODO Auto-generated constructor stub
//
//}
//
//ICM20948::~ICM20948() {
//	// TODO Auto-generated destructor stub
//}

uint8_t ICM20948::whoami(){
	    uint8_t adress=0;
		HAL_I2C_Mem_Read(hi2c, (uint16_t)address<<1, REGISTER.WHO_AM_I, 1, &adress, 1, 1000);
		return adress;

	}

	uint8_t ICM20948::pwrmgmt1(uint8_t data){
	     HAL_I2C_Mem_Write(hi2c, (uint16_t)address<<1,REGISTER.PWR_MGMT_1,1,&data,1,1000);
	     return 0;
	}
	uint8_t ICM20948::pwrmgmt2(uint8_t data){
	     HAL_I2C_Mem_Write(hi2c, (uint16_t)address<<1,REGISTER.PWR_MGMT_2,1,&data,1,1000);
	     return 0;
	}
	void ICM20948::reset(){
	    pwrmgmt1(ICM20948_BIT_H_RESET);
	    HAL_Delay(100);
	    pwrmgmt1(ICM20948_BTT_CLK_PLL);
	    HAL_Delay(100);
	    uint8_t buffer=ICM20948_BIT_INT_ACTL | ICM20948_BIT_INT_OPEN;
	    HAL_I2C_Mem_Write(hi2c, (uint16_t)address<<1,REGISTER.INT_PIN_CFG,1,&buffer,1,1000);
		}
	bool ICM20948::changeUserBank(const uint8_t bank){
	    if(bank>3 || bank<0){
	        return false;
	    }
	    uint8_t data=bank<<4;
	    uint8_t res = HAL_I2C_Mem_Write(hi2c, (uint16_t)address<<1,REGISTER.BANK_SEL,1,&data,1,1000);
	    //printf("data=0x%02x\n",data);
	    //printf("res=%d\n",res);
	    return true;
	}

	bool ICM20948::accelConfig(const AccelSensitivity fssel,const bool enableDLPF,const uint8_t configDLPF){

	    _accelsensitivity=fssel;
	    if(configDLPF>7 || configDLPF<0){
	        return false;
	    }
	    uint8_t data=0;
	    data |=configDLPF<<3;
	    data |=(uint8_t)fssel<<1;
	    data |=enableDLPF;
	    changeUserBank(2);

	    data=HAL_I2C_Mem_Write(hi2c, (uint16_t)address<<1,REGISTER.ACCEL_CONFIG,1,&data,1,1000);
	    //printf("data=%d\n",data);
	    changeUserBank(0);
	    return true;

	}
	bool ICM20948::gyroConfig(const GyroSensitivity fssel,const bool enableDLPF,const uint8_t configDLPF){

		_gyrosensitivity=fssel;
		if(configDLPF>7||configDLPF<0){
			return false;
		}
		uint8_t data=0;
		data |=configDLPF<<3;
		data |=(uint8_t)fssel<<1;
		data |=enableDLPF;
		changeUserBank(2);

		HAL_I2C_Mem_Write(hi2c,(uint16_t)address<<1,REGISTER.GYRO_CONFIG,1,&data,1,1000);
		changeUserBank(0);
		return true;

	}
	float ICM20948::getAccel(uint8_t axis){
	    const uint8_t REG_ACCEL_H[3]={0x2D,0x2F,0x31};
	    const uint8_t REG_ACCEL_L[3]={0x2E,0x30,0x32};

	    int8_t accelH;
		HAL_I2C_Mem_Read(hi2c, (uint16_t)address<<1,REG_ACCEL_H[axis],1,(uint8_t*)&accelH,1,1000);
	    int8_t accelL;
	    HAL_I2C_Mem_Read(hi2c, (uint16_t)address<<1,REG_ACCEL_L[axis],1,(uint8_t*)&accelL,1,1000);
	    int16_t accel=(int16_t)accelH<<8 | (int16_t)accelL;

	    return (float)accel/ACCEL_SENSITIVITY[(uint8_t)_accelsensitivity];
	}

	float ICM20948::getGyro(uint8_t axis){
		const uint8_t REG_GYRO_H[3]={0x33,0x35,0x37};
		const uint8_t REG_GYRO_L[3]={0x34,0x36,0x38};

		int8_t gyroH;
		HAL_I2C_Mem_Read(hi2c,(uint16_t)address<<1,REG_GYRO_H[axis],1,(uint8_t*)&gyroH,1,1000);
		int8_t gyroL;
		HAL_I2C_Mem_Read(hi2c,(uint16_t)address<<1,REG_GYRO_L[axis],1,(uint8_t*)&gyroL,1,1000);
		int16_t gyro=(int16_t)gyroH<<8 | (int16_t)gyroL;

		return (float)gyro/GYRO_SENSITIVITY[(uint8_t)_gyrosensitivity];
	}

	void ICM20948::getAccelBurst(std::array<float,3> &value){
		const uint8_t headRegAddr = 0x2d;
		uint8_t buffer[6]={};

		HAL_I2C_Mem_Read(hi2c, (uint16_t)address<<1,headRegAddr,1,(uint8_t*)buffer,6,1000);

		for(uint8_t n=0;n<3;n++){
			value[n] = (float)((int16_t)buffer[2*n]<<8 | (int16_t)buffer[2*n+1])/ACCEL_SENSITIVITY[(uint8_t)_accelsensitivity];
		}
	}

	void ICM20948::getGyroBurst(std::array<float,3> &value){
		const uint8_t headRegAddr = 0x33;
		uint8_t buffer[6]={};

		HAL_I2C_Mem_Read(hi2c, (uint16_t)address<<1,headRegAddr,1,(uint8_t*)buffer,6,1000);

		for(uint8_t n=0;n<3;n++){
			value[n] = (float)((int16_t)buffer[2*n]<<8 | (int16_t)buffer[2*n+1])/GYRO_SENSITIVITY[(uint8_t)_gyrosensitivity];
		}
	}
float debug;
int16_t ddd;
	void ICM20948::get6ValueBurst(std::array<float,3> &accel, std::array<float,3> &gyro){
		const uint8_t headRegAddr = 0x2d;
		uint8_t buffer[12]={};

		HAL_I2C_Mem_Read(hi2c, (uint16_t)address<<1,headRegAddr,1,buffer,12,1000);

		for(uint8_t n=0;n<3;n++){
			int16_t tmp = (int16_t)buffer[2*n]<<8 | (int16_t)buffer[2*n+1];
			accel[n] = (float)(tmp)/ACCEL_SENSITIVITY[(uint8_t)_accelsensitivity];
		}
		for(uint8_t n=0;n<3;n++){
			int16_t tmp = (int16_t)buffer[2*n+6]<<8 | (int16_t)buffer[2*n+1+6];
			gyro[n] = (float)(tmp)/GYRO_SENSITIVITY[(uint8_t)_gyrosensitivity];
		}
	}

	void ICM20948::intPinConfig(uint8_t value){
		HAL_I2C_Mem_Write(hi2c,(uint16_t)address<<1,REGISTER.INT_PIN_CFG,1,&value,1,1000);

	}

	void ICM20948::intenable(uint8_t value){
		HAL_I2C_Mem_Write(hi2c,(uint16_t)address<<1,REGISTER.INT_ENABLE,1,&value,1,1000);
	}
