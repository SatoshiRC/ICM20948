/*
 * ICM20948_raspi.cpp
 *
 *  Created on: Oct 21, 2025
 *      Author: GitHub Copilot
 */

#include "ICM20948_raspi.h"

#ifdef __linux__

#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <linux/i2c-dev.h>

ICM20948_raspi::~ICM20948_raspi(){
	end();
}

bool ICM20948_raspi::begin(){
	// Open I2C device
	i2c_fd = open(i2c_device_path, O_RDWR);
	if (i2c_fd < 0) {
		return false;
	}
	
	// Set I2C slave address
	if (ioctl(i2c_fd, I2C_SLAVE, (uint8_t)address) < 0) {
		close(i2c_fd);
		i2c_fd = -1;
		return false;
	}
	
	return true;
}

void ICM20948_raspi::end(){
	if (i2c_fd >= 0) {
		close(i2c_fd);
		i2c_fd = -1;
	}
}

void ICM20948_raspi::__memWrite(uint8_t memAddress, uint8_t *pData, uint8_t length){
	if (i2c_fd < 0) {
		return;
	}
	
	// Write register address followed by data
	uint8_t buffer[length + 1];
	buffer[0] = memAddress;
	for (uint8_t i = 0; i < length; i++) {
		buffer[i + 1] = pData[i];
	}
	
	write(i2c_fd, buffer, length + 1);
}

void ICM20948_raspi::__memRead(uint8_t memAddress, uint8_t *pData, uint8_t length){
	if (i2c_fd < 0) {
		return;
	}
	
	// Write register address
	write(i2c_fd, &memAddress, 1);
	
	// Read data
	read(i2c_fd, pData, length);
}

#endif /* __linux__ */
