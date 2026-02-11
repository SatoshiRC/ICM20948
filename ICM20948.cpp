/*
 * ICM20948.cpp
 *
 *  Created on: Oct 3, 2022
 *      Author: zeroi
 */

#include "ICM20948.h"

uint8_t ICM20948::whoami(){
	uint8_t adress=0;
	memRead(REGISTER::BANK0::WHO_AM_I, &adress);
	return adress;
}

void ICM20948::pwrmgmt1(uint8_t data){
	 memWrite(REGISTER::BANK0::PWR_MGMT_1, &data);
}

void ICM20948::pwrmgmt2(uint8_t data){
	memWrite(REGISTER::BANK0::PWR_MGMT_2, &data);
}

void ICM20948::reset(){
	uint8_t n=0b1<<7;
	memWrite(REGISTER::BANK0::PWR_MGMT_1, &n);
	__delay(10);
}

bool ICM20948::changeUserBank(REGISTER::BANK bank){
	uint8_t data=(uint8_t)bank<<4;
	__memWrite((uint8_t)REGISTER::BANK0::REG_BANK_SEL, &data);
	this->currentBank = bank;
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

	memWrite(REGISTER::BANK2::ACCEL_CONFIG, &data);

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

	memWrite(REGISTER::BANK2::GYRO_CONFIG_1, &data);
	return true;

}

void ICM20948::readAccel(){
	memRead(REGISTER::BANK0::ACCEL_XOUT_H, (uint8_t*)raw.data(),6);
	requireCalcAccel = true;
}

void ICM20948::readGyro(){
	memRead(REGISTER::BANK0::GYRO_XOUT_H, (uint8_t*)&raw[3],6);
	requireCalcGyro = true;
}

void ICM20948::readIMU(){
	memRead(REGISTER::BANK0::ACCEL_XOUT_H, (uint8_t*)raw.data(),12);
	requireCalcAccel = true;
	requireCalcGyro = true;
}

void ICM20948::readIMU_DMA(){
	memReadDma(REGISTER::BANK0::ACCEL_XOUT_H, (uint8_t*)raw.data(),12);
	requireCalcAccel = true;
	requireCalcGyro = true;
}

float ICM20948::getAccel(AXSIS axsis){
	if(requireCalcAccel){
		for(uint8_t n=0; n<3; n++){
			accel[n] = calculateAccel(raw[n]);
		}
		requireCalcAccel = false;
	}

	return accel[(uint8_t)axsis];
}

float ICM20948::getGyro(AXSIS axsis){
	if(requireCalcGyro){
		for(uint8_t n=0; n<3; n++){
			gyro[n] = calculateGyro(raw[n+3]);
		}
		requireCalcGyro = false;
	}

	return gyro[(uint8_t)axsis];
}

void ICM20948::getAccel(std::array<float,3> &value){
	if(requireCalcAccel){
		for(uint8_t n=0; n<3; n++){
			accel[n] = calculateAccel(raw[n]);
		}
		requireCalcAccel = false;
	}

	value = accel;
}

void ICM20948::getGyro(std::array<float,3> &value){
	if(requireCalcGyro){
		for(uint8_t n=0; n<3; n++){
			gyro[n] = calculateGyro(raw[n+3]);
		}
		requireCalcGyro = false;
	}
	value = gyro;
}

void ICM20948::getIMU(std::array<float,3> &accel, std::array<float,3> &gyro){
	getAccel(accel);
	getGyro(gyro);
}

void ICM20948::intPinConfig(uint8_t value){
	memWrite(REGISTER::BANK0::INT_PIN_CFG, &value);
}

void ICM20948::intenable1(uint8_t value){
	memWrite(REGISTER::BANK0::INT_ENABLE_1, &value);
}

void ICM20948::memWrite(REGISTER reg, uint8_t *pData, uint8_t length){
	if(this->currentBank != reg.bank){
		changeUserBank(reg.bank);
	}
	__memWrite(reg.address, pData, length);
}
void ICM20948::memRead(REGISTER reg, uint8_t *pData, uint8_t length){
	if(this->currentBank != reg.bank){
		changeUserBank(reg.bank);
	}
	__memRead(reg.address, pData, length);
}

void ICM20948::memReadDma(REGISTER reg, uint8_t *pData, uint8_t length){
	if(this->currentBank != reg.bank){
		changeUserBank(reg.bank);
	}
	__memReadDma(reg.address, pData, length);
}

float ICM20948::calculateAccel(const int16_t raw){
	return raw / ACCEL_SENSITIVITY[(uint8_t)_accelsensitivity];
}

float ICM20948::calculateGyro(const int16_t raw){
	return raw / GYRO_SENSITIVITY[(uint8_t)_gyrosensitivity];
}

float ICM20948::calculateMag(const int16_t raw){
	return raw * MAG_SENSITIVITY;
}

void ICM20948::writeMagRegister(uint8_t reg, uint8_t value){
	// Use I2C slave 0 for single register writes
	// Set slave 0 address to AK09916 in write mode
	memWrite(REGISTER::BANK3::I2C_SLV0_ADDR, AK09916::I2C_ADDR);
	// Set register address
	memWrite(REGISTER::BANK3::I2C_SLV0_REG, reg);
	// Set data to write
	memWrite(REGISTER::BANK3::I2C_SLV0_DO, value);
	// Enable I2C_SLV0 transaction: write 1 byte
	memWrite(REGISTER::BANK3::I2C_SLV0_CTRL, 0x80 | 1);
	__delay(10);
}

uint8_t ICM20948::readMagRegister(uint8_t reg){
	// Use I2C slave 0 for single register reads
	// Set slave 0 address to AK09916 in read mode
	memWrite(REGISTER::BANK3::I2C_SLV0_ADDR, AK09916::I2C_ADDR | 0x80);
	// Set register address
	memWrite(REGISTER::BANK3::I2C_SLV0_REG, reg);
	// Enable I2C_SLV0 transaction: read 1 byte
	memWrite(REGISTER::BANK3::I2C_SLV0_CTRL, 0x80 | 1);
	__delay(10);
	
	// Read data from EXT_SLV_SENS_DATA_00
	uint8_t data;
	memRead(REGISTER::BANK0::EXT_SLV_SENS_DATA_00, &data);
	return data;
}

bool ICM20948::initMagnetometer(){
	// Enable I2C master mode
	uint8_t userCtrl;
	memRead(REGISTER::BANK0::USER_CTRL, &userCtrl);
	userCtrl |= 0x20; // I2C_MST_EN
	memWrite(REGISTER::BANK0::USER_CTRL, userCtrl);
	
	// Configure I2C master clock to 400 kHz
	// I2C_MST_CLK = 400 kHz requires I2C_MST_CTRL = 7 (400 kHz)
	memWrite(REGISTER::BANK3::I2C_MST_CTRL, 0x07);
	__delay(1);
	
	// Reset magnetometer
	writeMagRegister((uint8_t)AK09916::REG::CNTL3, AK09916::CNTL3_RESET);
	__delay(10);
	
	// Check WHO_AM_I register
	uint8_t whoami = readMagRegister((uint8_t)AK09916::REG::WIA2);
	if(whoami != AK09916::WIA2_VALUE){
		return false;
	}
	
	// Set magnetometer to continuous measurement mode 100Hz
	writeMagRegister((uint8_t)AK09916::REG::CNTL2, AK09916::CNTL2_CONT_MODE_100HZ);
	__delay(10);
	
	// Configure I2C Slave 0 to read magnetometer data automatically
	// Set slave 0 address to AK09916 in read mode
	memWrite(REGISTER::BANK3::I2C_SLV0_ADDR, AK09916::I2C_ADDR | 0x80);
	// Start reading from ST1 register
	memWrite(REGISTER::BANK3::I2C_SLV0_REG, (uint8_t)AK09916::REG::ST1);
	// Read 9 bytes (ST1, HXL, HXH, HYL, HYH, HZL, HZH, dummy, ST2)
	memWrite(REGISTER::BANK3::I2C_SLV0_CTRL, 0x80 | 9); // Enable + 9 bytes
	
	return true;
}

void ICM20948::readMagnetometer(){
	// Read magnetometer data from EXT_SLV_SENS_DATA registers
	uint8_t magData[9];
	memRead(REGISTER::BANK0::EXT_SLV_SENS_DATA_00, magData, 9);
	
	// ST1 is at magData[0], data starts at magData[1]
	// Check if data is ready (ST1 bit 0)
	if(magData[0] & 0x01){
		// Check if data is not overrun (ST2 bit 3) 
		if(!(magData[8] & 0x08)){
			// Convert bytes to int16_t (little endian)
			rawMag[0] = (int16_t)((magData[2] << 8) | magData[1]); // X
			rawMag[1] = (int16_t)((magData[4] << 8) | magData[3]); // Y
			rawMag[2] = (int16_t)((magData[6] << 8) | magData[5]); // Z
			requireCalcMag = true;
		}
	}
}

void ICM20948::getMagnetometer(std::array<float,3> &value){
	if(requireCalcMag){
		for(uint8_t n=0; n<3; n++){
			mag[n] = calculateMag(rawMag[n]);
		}
		requireCalcMag = false;
	}
	value = mag;
}

float ICM20948::getMagnetometer(AXSIS axsis){
	if(requireCalcMag){
		for(uint8_t n=0; n<3; n++){
			mag[n] = calculateMag(rawMag[n]);
		}
		requireCalcMag = false;
	}
	return mag[(uint8_t)axsis];
}
