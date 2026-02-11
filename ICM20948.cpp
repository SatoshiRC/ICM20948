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
	memRead(REGISTER::BANK0::GYRO_XOUT_H, (uint8_t*)&raw[6],6);
	requireCalcGyro = true;
}

void ICM20948::readIMU(){
	memRead(REGISTER::BANK0::ACCEL_XOUT_H, (uint8_t*)raw.data(),12);
	requireCalcAccel = true;
	requireCalcGyro = true;
}

void ICM20948::readImuDma(){
	memReadDma(REGISTER::BANK0::ACCEL_XOUT_H, (uint8_t*)raw.data(),12);
	requireCalcAccel = true;
	requireCalcGyro = true;
}

float ICM20948::getAccel(AXSIS axsis){
	if(requireCalcAccel){
		for(uint8_t n=0; n<3; n++){
			accel[n] = calculateAccel(static_cast<int16_t>(static_cast<uint16_t>(raw[2*n]) << 8 | static_cast<uint16_t>(raw[2*n+1])));
		}
		requireCalcAccel = false;
	}

	return accel[(uint8_t)axsis];
}

float ICM20948::getGyro(AXSIS axsis){
	if(requireCalcGyro){
		for(uint8_t n=0; n<3; n++){
			gyro[n] = calculateGyro(static_cast<int16_t>(static_cast<uint16_t>(raw[2*n+6]) << 8 | static_cast<uint16_t>(raw[2*n+1+6])));
		}
		requireCalcGyro = false;
	}

	return gyro[(uint8_t)axsis];
}

void ICM20948::getAccel(std::array<float,3> &value){
	if(requireCalcAccel){
		for(uint8_t n=0; n<3; n++){
			accel[n] = calculateAccel(static_cast<int16_t>(static_cast<uint16_t>(raw[2*n]) << 8 | static_cast<uint16_t>(raw[2*n+1])));
		}
		requireCalcAccel = false;
	}

	value = accel;
}

void ICM20948::getGyro(std::array<float,3> &value){
	if(requireCalcGyro){
		for(uint8_t n=0; n<3; n++){
			gyro[n] = calculateGyro(static_cast<int16_t>(static_cast<uint16_t>(raw[2*n+6]) << 8 | static_cast<uint16_t>(raw[2*n+1+6])));
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

float ICM20948::calculateMagnetometer(const int16_t raw){
	return raw * MAG_SENSITIVITY;
}

void ICM20948::processMagnetometerData(){
	// Check ST1 register bit 0 (DRDY)
	if(mag_raw[0] & AK09916_DRDY_BIT){
		// Data is ready, process it
		// ST2 register is at mag_raw[7], check for overflow
		if(!(mag_raw[7] & AK09916_OVERFLOW_BIT)){
			// No overflow, calculate magnetic field
			for(uint8_t n=0; n<3; n++){
				// Combine low and high bytes (little-endian format)
				int16_t raw_val = static_cast<int16_t>(static_cast<uint16_t>(mag_raw[2*n+1]) | (static_cast<uint16_t>(mag_raw[2*n+2]) << 8));
				mag[n] = calculateMagnetometer(raw_val);
			}
		}
	}
}

bool ICM20948::initMagnetometer(){
	// Enable I2C master mode
	uint8_t userCtrl = I2C_MST_EN;
	memWrite(REGISTER::BANK0::USER_CTRL, &userCtrl);
	
	// Configure I2C master clock to 400kHz
	uint8_t i2cMstCtrl = I2C_MST_CLK_400KHZ;
	memWrite(REGISTER::BANK3::I2C_MST_CTRL, &i2cMstCtrl);
	
	// Wait for I2C master to be ready
	__delay(10);
	
	// Reset AK09916
	uint8_t ak09916Addr = AK09916_ADDRESS; // Write mode (R/W bit = 0)
	memWrite(REGISTER::BANK3::I2C_SLV0_ADDR, &ak09916Addr);
	uint8_t ak09916Reg = AK09916_CNTL3;
	memWrite(REGISTER::BANK3::I2C_SLV0_REG, &ak09916Reg);
	uint8_t resetCmd = AK09916_SRST;
	memWrite(REGISTER::BANK3::I2C_SLV0_DO, &resetCmd);
	uint8_t slv0Ctrl = I2C_SLV0_EN_1_BYTE;
	memWrite(REGISTER::BANK3::I2C_SLV0_CTRL, &slv0Ctrl);
	
	// Wait for magnetometer reset to complete
	__delay(10);
	
	// Set AK09916 to continuous measurement mode 4 (100Hz, register value 0x08)
	ak09916Reg = AK09916_CNTL2;
	memWrite(REGISTER::BANK3::I2C_SLV0_REG, &ak09916Reg);
	uint8_t modeCmd = AK09916_MODE_CONTINUOUS_100HZ;
	memWrite(REGISTER::BANK3::I2C_SLV0_DO, &modeCmd);
	slv0Ctrl = I2C_SLV0_EN_1_BYTE;
	memWrite(REGISTER::BANK3::I2C_SLV0_CTRL, &slv0Ctrl);
	
	// Wait for mode change to take effect
	__delay(10);
	
	// Configure SLV0 to read magnetometer data
	ak09916Addr = AK09916_ADDRESS | I2C_SLV_READ_FLAG; // Read mode (R/W bit = 1)
	memWrite(REGISTER::BANK3::I2C_SLV0_ADDR, &ak09916Addr);
	ak09916Reg = AK09916_STATUS1;
	memWrite(REGISTER::BANK3::I2C_SLV0_REG, &ak09916Reg);
	slv0Ctrl = I2C_SLV0_EN_8_BYTES; // ST1, HXL, HXH, HYL, HYH, HZL, HZH, ST2
	memWrite(REGISTER::BANK3::I2C_SLV0_CTRL, &slv0Ctrl);
	
	return true;
}

void ICM20948::readMagnetometer(){
	memRead(REGISTER::BANK0::EXT_SLV_SENS_DATA_00, (uint8_t*)mag_raw.data(), 8);
	requireCalcMag = true;
}

void ICM20948::getMagnetometer(std::array<float,3> &value){
	if(requireCalcMag){
		processMagnetometerData();
		requireCalcMag = false;
	}
	value = mag;
}

float ICM20948::getMagnetometer(AXSIS axsis){
	if(requireCalcMag){
		processMagnetometerData();
		requireCalcMag = false;
	}
	return mag[(uint8_t)axsis];
}
