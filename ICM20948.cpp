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
	memRead(REGISTER::BANK0::ACCEL_XOUT_H, (uint8_t*)raw.data(),raw.size());
	requireCalcAccel = true;
	requireCalcGyro = true;
	requireCalcMag = true;
}

void ICM20948::readImuDma(){
	memReadDma(REGISTER::BANK0::ACCEL_XOUT_H, (uint8_t*)raw.data(),raw.size());
	requireCalcAccel = true;
	requireCalcGyro = true;
	requireCalcMag = true;
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
	std::array<uint8_t, 9> mag_raw;
	std::copy(raw.data()+14, raw.end(), mag_raw.data());
	if(mag_raw[0] > 0){
		// Data is ready, process it
		// ST2 register is at mag_raw[8], check for overflow
		if(!(mag_raw[8] & AK09916_OVERFLOW_BIT)){
			// No overflow, calculate magnetic field
			for(uint8_t n=0; n<3; n++){
				// Combine low and high bytes (little-endian format)
				magRaw[n] = static_cast<int16_t>(static_cast<uint16_t>(mag_raw[2*n+1]) | (static_cast<uint16_t>(mag_raw[2*n+2]) << 8));
				mag[n] = calculateMagnetometer(magRaw[n]);
			}
		}
	}
}

bool ICM20948::initMagnetometer(MagnetometerMode mode){
	// Ensure we're in BANK0
	changeUserBank(REGISTER::BANK::BANK0);
	
	// Enable I2C master mode
	uint8_t userCtrl;
	memRead(REGISTER::BANK0::USER_CTRL, &userCtrl);
	userCtrl |= I2C_MST_EN;
	memWrite(REGISTER::BANK0::USER_CTRL, &userCtrl);
	__delay(100);
	
	// Switch to BANK3 for I2C Master control
	changeUserBank(REGISTER::BANK::BANK3);
	
	// Configure I2C master clock & enable multi-master
	uint8_t i2cMstCtrl = I2C_MST_CLK_400KHZ | 0x20;  // 0x20: wait for external signal
	memWrite(REGISTER::BANK3::I2C_MST_CTRL, &i2cMstCtrl);
	__delay(100);

//
	// Reset AK09916 via SLV0 (write mode)
	uint8_t slvAddr = AK09916_ADDRESS;  // Write address
	memWrite(REGISTER::BANK3::I2C_SLV4_ADDR, &slvAddr);

	uint8_t slvReg = AK09916_CNTL3;
	memWrite(REGISTER::BANK3::I2C_SLV4_REG, &slvReg);

	uint8_t resetVal = AK09916_SRST;
	memWrite(REGISTER::BANK3::I2C_SLV4_DO, &resetVal);

	uint8_t slvCtrl = 0x80;
	memWrite(REGISTER::BANK3::I2C_SLV4_CTRL, &slvCtrl);
	__delay(200);
	
	// Set measurement mode
	setMagnetometerMode(mode);
	/*
	uint8_t slvAddr = AK09916_ADDRESS;  // Write address
	memWrite(REGISTER::BANK3::I2C_SLV4_ADDR, &slvAddr);

	uint8_t ak09916Reg = AK09916_CNTL2;
	memWrite(REGISTER::BANK3::I2C_SLV4_REG, &ak09916Reg);
	uint8_t modeCmd = static_cast<uint8_t>(mode);
	memWrite(REGISTER::BANK3::I2C_SLV4_DO, &modeCmd);
	uint8_t slv0Ctrl = 0x80;
	memWrite(REGISTER::BANK3::I2C_SLV4_CTRL, &slv0Ctrl);

	// Wait for mode change to take effect
	__delay(100);
	 */
	__delay(200);
	
	// Disable SLV0 first (to clear any pending state)
	uint8_t disable = 0x00;
	memWrite(REGISTER::BANK3::I2C_SLV0_CTRL, &disable);
	memWrite(REGISTER::BANK3::I2C_SLV0_DO, &disable); // DOをクリアしておく！
	__delay(50);
	
	uint8_t odr_conf = 0x05;
	memWrite(REGISTER::BANK3::I2C_MST_ODR_CONFIG, &odr_conf);


	// Re-enable SLV0 for continuous reading (read mode)
	slvAddr = AK09916_ADDRESS | I2C_SLV_READ_FLAG;  // Read address
	memWrite(REGISTER::BANK3::I2C_SLV0_ADDR, &slvAddr);
	
	slvReg = AK09916_STATUS1;  // Start from ST1 register
	memWrite(REGISTER::BANK3::I2C_SLV0_REG, &slvReg);
	
	// Enable reading 8 bytes: ST1, HXL, HXH, HYL, HYH, HZL, HZH, RESERVE ,ST2
	slvCtrl = I2C_SLV0_EN_8_BYTES;
	memWrite(REGISTER::BANK3::I2C_SLV0_CTRL, &slvCtrl);
	__delay(200);
	
	// Return to BANK0
	changeUserBank(REGISTER::BANK::BANK0);
	
	return true;
}

void ICM20948::setMagnetometerMode(MagnetometerMode mode){
	// Set I2C_SLV0_ADDR to write mode (without read flag)
	uint8_t slvAddr = AK09916_ADDRESS;  // Write address
	memWrite(REGISTER::BANK3::I2C_SLV4_ADDR, &slvAddr);
	
	uint8_t ak09916Reg = AK09916_CNTL2;
	memWrite(REGISTER::BANK3::I2C_SLV4_REG, &ak09916Reg);
	uint8_t modeCmd = static_cast<uint8_t>(mode);
	memWrite(REGISTER::BANK3::I2C_SLV4_DO, &modeCmd);
	uint8_t slv0Ctrl = 0x80;
	memWrite(REGISTER::BANK3::I2C_SLV4_CTRL, &slv0Ctrl);
	
	// Wait for mode change to take effect
	__delay(100);
}

void ICM20948::readMagnetometer(){
	memRead(REGISTER::BANK0::EXT_SLV_SENS_DATA_00, (uint8_t*)raw.data() + 14, 8);
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
