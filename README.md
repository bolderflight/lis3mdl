[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)

![Bolder Flight Systems Logo](img/logo-words_75.png) &nbsp; &nbsp; ![Arduino Logo](img/arduino_logo_75.png)

# Lis3mdl
This library communicates with the [LIS3MDL](https://www.st.com/en/mems-and-sensors/lis3mdl.html) magnetometer. This library is compatible with Arduino ARM and CMake build systems.
   * [License](LICENSE.md)
   * [Changelog](CHANGELOG.md)
   * [Contributing guide](CONTRIBUTING.md)

# Description
The Invense LIS3MDL is a three-axis magnetometer, which supports I2C, up to 400 kHz, and SPI communication, up to 10 MHz. The following selectable full scale sensor ranges and output data rates are available:

| Full Scale Range |
| --- |
| +/- 4 Gauss |
| +/- 8 Gauss |
| +/- 12 Gauss |
| +/- 16 Gauss |

| Output Data Rate |
| --- |
| 0.625HZ |
| 1.25HZ |
| 2.5HZ |
| 5HZ |
| 10HZ |
| 20HZ |
| 40HZ |
| 80HZ |
| 155HZ |
| 300HZ |
| 560HZ |
| 1000HZ |

# Installation

## Arduino
Use the Arduino Library Manager to install this library or clone to your Arduino/libraries folder. This library is added as:

```C++
#include "lis3mdl.h"
```

Example Arduino executables are located in: *examples/arduino/*. Teensy 3.x, 4.x, and LC devices are used for testing under Arduino and this library should be compatible with other Arduino devices.

## CMake
CMake is used to build this library, which is exported as a library target called *lis3mdl*. This library is added as:

```C++
#include "lis3mdl.h"
```

The library can be also be compiled stand-alone using the CMake idiom of creating a *build* directory and then, from within that directory issuing:

```
cmake .. -DMCU=MK66FX1M0
make
```

This will build the library and example executables called *i2c_example* and *spi_example*. The example executable source files are located at *examples/cmake*. Notice that the *cmake* command includes a define specifying the microcontroller the code is being compiled for. This is required to correctly configure the code, CPU frequency, and compile/linker options. The available MCUs are:
   * MK20DX128
   * MK20DX256
   * MK64FX512
   * MK66FX1M0
   * MKL26Z64
   * IMXRT1062_T40
   * IMXRT1062_T41
   * IMXRT1062_MMOD

These are known to work with the same packages used in Teensy products. Also switching packages is known to work well, as long as it's only a package change.

The example targets create executables for communicating with the sensor using I2C or SPI communication, using the data ready interrupt, and using the wake on motion interrupt, respectively. Each target also has a *_hex*, for creating the hex file to upload to the microcontroller, and an *_upload* for using the [Teensy CLI Uploader](https://www.pjrc.com/teensy/loader_cli.html) to flash the Teensy. Please note that instructions for setting up your build environment can be found in our [build-tools repo](https://github.com/bolderflight/build-tools).

# Namespace
This library is within the namespace *bfs*.

# Lis3mdl

## Methods

**Lis3mdl()** Default constructor, requires calling the Config method to setup the I2C or SPI bus and I2C address or SPI chip select pin.

**Lis3mdl(TwoWire &ast;i2c, const I2cAddr addr)** Creates a *Lis3mdl* object using I2C communication. A pointer to the bus is passed along with the I2C address. The primary address (0x1C) should be used if SDO is grounded and the secondary address (0x1E) if SDO is pulled high.

```C++
bfs::Lis3mdl mag(&Wire, bfs::Lis3mdl::I2C_ADDR_PRIM);
```

**Lis3mdl(SPIClass &ast;spi, const uint8_t cs)** Creates a *Lis3mdl* object using SPI communication. A pointer to the bus is passed along with the chip select pin number. Any digital pin can be used as chip select.

```C++
bfs::Lis3mdl mag(&SPI, 10);
```

**void Config(TwoWire &ast;bus, const I2cAddr addr)** This is required when using the default constructor and sets up the I2C bus and I2C address.

**void Config(SPIClass &ast;spi, const uint8_t cs)** This is required when using the default constructor and sets up the SPI bus and chip select pin.

**bool Begin()** Initializes communication with the sensor and sets a default configuration. True is returned on success and false on failure. Note that the bus is not initialized in this method to improve compatibility with other sensors on the bus. The default configuration is:

| Range | Output Data Rate |
| --- | --- |
| +/-16 Gauss | 155 Hz |

```C++
Wire.begin();
Wire.setClock(400000);
if (!mag.Begin()) {
  // error handling
}
```

**bool ConfigOdr(const Odr odr)** Configures the output data rate. True is returned on success and false on failure. The available options are:

| Enum | Rate |
| --- | --- |
| ODR_0_625HZ | 0.625Hz |
| ODR_1_25HZ | 1.25Hz |
| ODR_2_5HZ | 2.5Hz |
| ODR_5HZ | 5HZ |
| ODR_10HZ | 10HZ |
| ODR_20HZ | 20HZ |
| ODR_40HZ | 40HZ |
| ODR_80HZ | 80HZ |
| ODR_155HZ | 155HZ |
| ODR_300HZ | 300HZ |
| ODR_560HZ | 560HZ |
| ODR_1000HZ | 1000HZ |

```C++
if(!mag.ConfigOdr(bfs::Lis3mdl::ODR_10HZ)) {
  // error handling
}
```

**Odr odr()** Returns the current output data rate.

**bool ConfigRange(const Range range)** Configures the sensor full scale range. True is returned on success and false on failure. The available options are:

| Enum | Range |
| --- | --- |
| RANGE_4GS | +/- 4 Gauss |
| RANGE_8GS | +/- 8 Gauss |
| RANGE_12GS | +/- 12 Gauss |
| RANGE_16GS | +/- 16 Gauss |

```C++
if(!mag.ConfigRange(bfs::Lis3mdl::RANGE_12GS)) {
  // error handling
}
```

**Range range()** Returns the current full scale range.

**bool Read()** Reads data from the sensor and stores the data in the *Lis3mdl* object. Note that new data from each axis is checked individually. If data from any axis is updated, this method returns true. When using the data ready interrupt to call this method, all three axes are typically updated simultaneously. When polling this method, typically x and y are updated each frame at the specified output data rate. The z axis data sometimes misses a frame.

```C++
if (mag.Read()) {
  // data processing
}
```

**bool new_x_data()** Returns true if the x axis data has been updated. This flag is latched and reset when this method is called. Similar methods exist for y and z axis.

```C++
bool new_data[3];
new_data[0] = mag.new_x_data();
new_data[1] = mag.new_y_data();
new_data[2] = mag.new_z_data();
```

**float mag_x_ut()** Returns the most recent x axis measurement from the *Lis3mdl* object in uT. Similar methods exist for y and z axis.

```C++
float data[3];
data[0] = mag.mag_x_ut();
data[1] = mag.mag_y_ut();
data[2] = mag.mag_z_ut();
```
