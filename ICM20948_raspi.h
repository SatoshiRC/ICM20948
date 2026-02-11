/*
 * ICM20948_raspi.h
 *
 *  Created on: Oct 21, 2025
 *      Author: GitHub Copilot
 */

#ifndef ICM20948_ICM20948_RASPI_H_
#define ICM20948_ICM20948_RASPI_H_

#ifdef __linux__

#include "ICM20948.h"

class ICM20948_raspi: public ICM20948 {
public:
	ICM20948_raspi(const char* i2c_device, Address address)
	:ICM20948(address), i2c_fd(-1), i2c_device_path(i2c_device){}
	
	~ICM20948_raspi();
	
	bool begin();
	void end();
	
	int getI2CFd() const {
		return i2c_fd;
	}

private:
	int i2c_fd;
	const char* i2c_device_path;

	void __memWrite(uint8_t memAddress, uint8_t *pData, uint8_t length=1);
	void __memRead(uint8_t memAddress, uint8_t *pData, uint8_t length=1);
	void __memReadDma(uint8_t memAddress, uint8_t *pData, uint8_t length=1);
	void __delay(uint32_t ms);
};

#endif /* __linux__ */

#endif /* ICM20948_ICM20948_RASPI_H_ */
