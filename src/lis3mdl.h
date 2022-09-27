/*
* Brian R Taylor
* brian.taylor@bolderflight.com
* 
* Copyright (c) 2022 Bolder Flight Systems Inc
*
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the “Software”), to
* deal in the Software without restriction, including without limitation the
* rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
* sell copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in
* all copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED “AS IS”, WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
* AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
* FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
* IN THE SOFTWARE.
*/

#ifndef LIS3MDL_SRC_LIS3MDL_H_  // NOLINT
#define LIS3MDL_SRC_LIS3MDL_H_

#if defined(ARDUINO)
#include <Arduino.h>
#include "Wire.h"
#include "SPI.h"
#else
#include <cstddef>
#include <cstdint>
#include "core/core.h"
#endif

namespace bfs {

class Lis3mdl {
 public:
  enum I2cAddr : uint8_t {
    I2C_ADDR_PRIM = 0x1C,
    I2C_ADDR_SEC = 0x1E
  };
  enum Range : uint8_t {
    RANGE_4GS =   0b00,
    RANGE_8GS =   0b01,
    RANGE_12GS =  0b10,
    RANGE_16GS =  0b11
  };
  enum Odr : uint8_t {
    ODR_0_625HZ = 0b0000,
    ODR_1_25HZ =  0b0010,
    ODR_2_5HZ =   0b0100,
    ODR_5HZ =     0b0110,
    ODR_10HZ =    0b1000,
    ODR_20HZ =    0b1010,
    ODR_40HZ =    0b1100,
    ODR_80HZ =    0b1110,
    ODR_155HZ =   0b0001,
    ODR_300HZ =   0b0011,
    ODR_560HZ =   0b0101,
    ODR_1000HZ =  0b0111
  };
  Lis3mdl() {}
  Lis3mdl(TwoWire *i2c, const I2cAddr addr) : i2c_(i2c),
                                              dev_(static_cast<uint8_t>(addr)),
                                              iface_(I2C) {}
  Lis3mdl(SPIClass *spi, const uint8_t cs) : spi_(spi), dev_(cs), iface_(SPI) {}
  void Config(TwoWire *i2c, const I2cAddr addr);
  void Config(SPIClass *spi, const uint8_t cs);
  bool Begin();
  bool ConfigOdr(const Odr odr);
  inline Odr odr() const {return odr_;}
  bool ConfigRange(const Range range);
  inline Range range() const {return range_;}
  bool Read();
  bool new_x_data();
  bool new_y_data();
  bool new_z_data();
  inline float mag_x_ut() const {return mag_ut_[0];}
  inline float mag_y_ut() const {return mag_ut_[1];}
  inline float mag_z_ut() const {return mag_ut_[2];}

 private:
  /* Comms */
  enum Interface : int8_t {
    SPI,
    I2C
  };
  TwoWire *i2c_;
  SPIClass *spi_;
  uint8_t dev_;
  Interface iface_;
  uint8_t bytes_rx_;
  static constexpr int32_t SPI_CLOCK_ = 7000000;
  static constexpr uint8_t SPI_READ_ = 0x80;
  static constexpr uint8_t SPI_MS_ = 0x40;
  static constexpr uint8_t LIS3MDL_WHOAMI_ = 0x3D;
  /* Config */
  enum PerfMode {
    PERF_MODE_LOW_POWER = 0b00,
    PERF_MODE_MEDIUM_POWER = 0b01,
    PERF_MODE_HIGH = 0b10,
    PERF_MODE_ULTRA_HIGH = 0b11
  };
  enum OpMode {
    OP_MODE_CONTINUOUS = 0b00,
    OP_MODE_SINGLE = 0b01,
    OP_MODE_POWERDOWN = 0b11
  };
  bool continuous_;
  uint8_t val_, data_, mask_;
  uint8_t who_am_i_;
  PerfMode perf_mode_;
  OpMode op_mode_;
  Odr odr_;
  Range range_;
  /* Data */
  uint8_t stat_;
  uint8_t buf_[9];
  int16_t x_, y_, z_;
  bool data_updated_;
  bool new_x_data_, new_y_data_, new_z_data_;
  float mag_ut_[3];
  float scale_;
  /* Register addresses */
  static constexpr uint8_t REG_WHO_AM_I_ = 0x0F;
  static constexpr uint8_t REG_CTRL_REG1_ = 0x20;
  static constexpr uint8_t REG_CTRL_REG2_ = 0x21;
  static constexpr uint8_t REG_CTRL_REG3_ = 0x22;
  static constexpr uint8_t REG_CTRL_REG4_ = 0x23;
  static constexpr uint8_t REG_CTRL_REG5_ = 0x24;
  static constexpr uint8_t REG_STATUS_REG_ = 0x27;
  static constexpr uint8_t REG_OUT_X_L_ = 0x28;
  static constexpr uint8_t REG_OUT_X_H_ = 0x29;
  static constexpr uint8_t REG_OUT_Y_L_ = 0x2A;
  static constexpr uint8_t REG_OUT_Y_H_ = 0x2B;
  static constexpr uint8_t REG_OUT_Z_L_ = 0x2C;
  static constexpr uint8_t REG_OUT_Z_H_ = 0x2D;
  static constexpr uint8_t REG_TEMP_OUT_L_ = 0x2E;
  static constexpr uint8_t REG_TEMP_OUT_H_ = 0x2F;
  static constexpr uint8_t REG_INT_CFG_ = 0x30;
  static constexpr uint8_t REG_INT_SRC_ = 0x31;
  static constexpr uint8_t REG_INT_THS_L_ = 0x32;
  static constexpr uint8_t REG_INT_THS_H_ = 0x33;
  /* Util functions */
  bool EnableBdu();
  bool EnableContinousMode(bool en);
  bool EnableTemp();
  bool SetPerfMode(const PerfMode perf_mode);
  bool SetOpMode(const OpMode op_mode);
  bool SetDataRate(const Odr data_rate);
  bool SetRange(const Range range);
  bool GetPerfMode(PerfMode * const perf_mode);
  bool GetOpMode(OpMode * const op_mode);
  bool GetDataRate(Odr * const data_rate);
  bool GetRange(Range * const range);
  bool Read(const uint8_t reg, const uint8_t bits, const uint8_t shift,
            uint8_t * const data);
  bool Write(const uint8_t reg, const uint8_t data, const uint8_t bits,
             const uint8_t shift);
  bool WriteRegister(const uint8_t reg, const uint8_t data);
  bool ReadRegisters(const uint8_t reg, const uint8_t count,
                     uint8_t * const data);
};

}  // namespace bfs

#endif  // LIS3MDL_SRC_LIS3MDL_H_ NOLINT
