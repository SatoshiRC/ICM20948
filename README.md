# ICM20948 Library

A C++ library for the ICM-20948 9-axis motion tracking device (accelerometer, gyroscope, and magnetometer).

## Supported Platforms

- **STM32 HAL**: Implementation using STM32 HAL I2C drivers (`ICM20948_HAL`)
- **Raspberry Pi / Linux**: Implementation using Linux I2C interface (`ICM20948_raspi`)

## Usage

### Raspberry Pi / Linux

1. **Enable I2C on Raspberry Pi**:
   ```bash
   sudo raspi-config
   # Navigate to: Interface Options -> I2C -> Enable
   ```

2. **Install required tools** (if needed):
   ```bash
   sudo apt-get install i2c-tools
   ```

3. **Check I2C device**:
   ```bash
   i2cdetect -y 1
   ```
   The ICM20948 should appear at address 0x68 (LOW) or 0x69 (HIGH).

4. **Include the library in your project**:
   ```cpp
   #include "ICM20948_raspi.h"
   ```

5. **Example code**:
   ```cpp
   // Create ICM20948 instance
   ICM20948_raspi imu("/dev/i2c-1", ICM20948::Address::LOW);
   
   // Initialize
   if (!imu.begin()) {
       std::cerr << "Failed to initialize" << std::endl;
       return 1;
   }
   
   // Configure
   imu.reset();
   imu.pwrmgmt1(imu.BTT_CLK_PLL);
   imu.pwrmgmt2(imu.ENABLE_SENSORS);
   imu.accelConfig(ICM20948::AccelSensitivity::SENS_2G, true, 0);
   imu.gyroConfig(ICM20948::GyroSensitivity::SENS_250, true, 0);
   
   // Initialize magnetometer
   imu.initMagnetometer();
   
   // Read data
   imu.readIMU();
   imu.readMagnetometer();
   std::array<float, 3> accel, gyro, mag;
   imu.getAccel(accel);
   imu.getGyro(gyro);
   imu.getMagnetometer(mag);
   
   // Clean up
   imu.end();
   ```

6. **Compile**:
   ```bash
   g++ -std=c++11 -o example example_raspi.cpp ICM20948.cpp ICM20948_raspi.cpp
   ```

7. **Run**:
   ```bash
   ./example
   ```

### STM32 HAL

1. **Include the library in your project**:
   ```cpp
   #include "ICM20948_HAL.h"
   ```

2. **Example code**:
   ```cpp
   extern I2C_HandleTypeDef hi2c1; // Your I2C handle
   
   ICM20948_HAL imu(&hi2c1, ICM20948::Address::LOW);
   
   // Use the same API as the Raspberry Pi version
   imu.reset();
   imu.pwrmgmt1(imu.BTT_CLK_PLL);
   // ...
   ```

## API Reference

### Class: ICM20948

Base class with common functionality.

#### Accelerometer Sensitivity
- `SENS_2G`: ±2g
- `SENS_4G`: ±4g
- `SENS_8G`: ±8g
- `SENS_16G`: ±16g

#### Gyroscope Sensitivity
- `SENS_250`: ±250 dps
- `SENS_500`: ±500 dps
- `SENS_1000`: ±1000 dps
- `SENS_2000`: ±2000 dps

#### I2C Address
- `Address::LOW`: 0x68 (AD0 pin low)
- `Address::HIGH`: 0x69 (AD0 pin high)

#### Methods
- `whoami()`: Read WHO_AM_I register (should return 0xEA)
- `reset()`: Reset the device
- `pwrmgmt1(uint8_t data)`: Configure power management 1
- `pwrmgmt2(uint8_t data)`: Configure power management 2
- `accelConfig(AccelSensitivity, bool enableDLPF, uint8_t configDLPF)`: Configure accelerometer
- `gyroConfig(GyroSensitivity, bool enableDLPF, uint8_t configDLPF)`: Configure gyroscope
- `readAccel()`: Read raw accelerometer data
- `readGyro()`: Read raw gyroscope data
- `readIMU()`: Read both accelerometer and gyroscope data
- `getAccel(AXSIS)`: Get accelerometer value for specific axis (in g)
- `getGyro(AXSIS)`: Get gyroscope value for specific axis (in rad/s)
- `getAccel(std::array<float,3>&)`: Get all accelerometer values (in g)
- `getGyro(std::array<float,3>&)`: Get all gyroscope values (in rad/s)
- `getIMU(std::array<float,3>&, std::array<float,3>&)`: Get both accelerometer and gyroscope values
- `initMagnetometer()`: Initialize the AK09916 magnetometer via I2C master interface (SLV0)
- `readMagnetometer()`: Read raw magnetometer data
- `getMagnetometer(AXSIS)`: Get magnetometer value for specific axis (in µT)
- `getMagnetometer(std::array<float,3>&)`: Get all magnetometer values (in µT)

### Class: ICM20948_raspi (Raspberry Pi / Linux)

Inherits from ICM20948.

#### Constructor
```cpp
ICM20948_raspi(const char* i2c_device, Address address)
```
- `i2c_device`: Path to I2C device (e.g., "/dev/i2c-1")
- `address`: I2C address (LOW or HIGH)

#### Methods
- `begin()`: Initialize I2C communication (returns true on success)
- `end()`: Close I2C connection

### Class: ICM20948_HAL (STM32)

Inherits from ICM20948.

#### Constructor
```cpp
ICM20948_HAL(I2C_HandleTypeDef *hi2c, Address address)
```
- `hi2c`: Pointer to STM32 HAL I2C handle
- `address`: I2C address (LOW or HIGH)

## Hardware Connections

### Raspberry Pi
| ICM20948 | Raspberry Pi |
|----------|--------------|
| VCC      | 3.3V         |
| GND      | GND          |
| SDA      | GPIO2 (SDA)  |
| SCL      | GPIO3 (SCL)  |
| AD0      | GND (for 0x68) or 3.3V (for 0x69) |

### STM32
Configure I2C peripheral in STM32CubeMX and connect:
| ICM20948 | STM32 |
|----------|-------|
| VCC      | 3.3V  |
| GND      | GND   |
| SDA      | I2C_SDA pin |
| SCL      | I2C_SCL pin |
| AD0      | GND (for 0x68) or 3.3V (for 0x69) |

## License

See LICENSE file for details.

## Authors

- Original ICM20948 implementation: zeroi
- STM32 HAL implementation: satoshi
- Raspberry Pi implementation: GitHub Copilot (2025)
