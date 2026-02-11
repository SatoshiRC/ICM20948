/*
 * example_raspi.cpp
 *
 * Example usage of ICM20948_raspi for Raspberry Pi
 * 
 * Compile: g++ -std=c++11 -o example example_raspi.cpp ICM20948.cpp ICM20948_raspi.cpp
 * Run: ./example
 */

#include "ICM20948_raspi.h"
#include <iostream>
#include <unistd.h>

int main() {
	// Create ICM20948 instance for I2C bus 1 with LOW address
	// Use "/dev/i2c-1" for Raspberry Pi (default I2C bus)
	// Use ICM20948::Address::LOW (0x68) or ICM20948::Address::HIGH (0x69)
	ICM20948_raspi imu("/dev/i2c-1", ICM20948::Address::LOW);
	
	// Initialize I2C communication
	if (!imu.begin()) {
		std::cerr << "Failed to initialize ICM20948" << std::endl;
		return 1;
	}
	
	// Check WHO_AM_I register (should return 0xEA)
	uint8_t whoami = imu.whoami();
	std::cout << "WHO_AM_I: 0x" << std::hex << (int)whoami << std::dec << std::endl;
	
	if (whoami != 0xEA) {
		std::cerr << "Invalid WHO_AM_I response. Expected 0xEA" << std::endl;
		return 1;
	}
	
	// Reset the device
	imu.reset();
	usleep(100000); // Wait 100ms after reset
	
	// Configure power management
	imu.pwrmgmt1(imu.BTT_CLK_PLL);
	imu.pwrmgmt2(imu.ENABLE_SENSORS);
	
	// Configure accelerometer: ±2g, DLPF enabled, config 0
	imu.accelConfig(ICM20948::AccelSensitivity::SENS_2G, true, 0);
	
	// Configure gyroscope: ±250dps, DLPF enabled, config 0
	imu.gyroConfig(ICM20948::GyroSensitivity::SENS_250, true, 0);
	
	// Initialize magnetometer
	if (!imu.initMagnetometer()) {
		std::cerr << "Failed to initialize magnetometer" << std::endl;
		return 1;
	}
	
	std::cout << "ICM20948 initialized successfully (with magnetometer)" << std::endl;
	
	// Read and display sensor data
	for (int i = 0; i < 10; i++) {
		// Read IMU data (both accelerometer and gyroscope)
		imu.readIMU();
		
		// Read magnetometer data
		imu.readMagnetometer();
		
		// Get processed accelerometer data in g
		std::array<float, 3> accel;
		imu.getAccel(accel);
		
		// Get processed gyroscope data in rad/s
		std::array<float, 3> gyro;
		imu.getGyro(gyro);
		
		// Get processed magnetometer data in uT
		std::array<float, 3> mag;
		imu.getMagnetometer(mag);
		
		std::cout << "Accel (g): "
		          << "X=" << accel[0] << " "
		          << "Y=" << accel[1] << " "
		          << "Z=" << accel[2] << " | ";
		
		std::cout << "Gyro (rad/s): "
		          << "X=" << gyro[0] << " "
		          << "Y=" << gyro[1] << " "
		          << "Z=" << gyro[2] << " | ";
		
		std::cout << "Mag (uT): "
		          << "X=" << mag[0] << " "
		          << "Y=" << mag[1] << " "
		          << "Z=" << mag[2] << std::endl;
		
		usleep(100000); // Wait 100ms between readings
	}
	
	// Clean up
	imu.end();
	
	return 0;
}
