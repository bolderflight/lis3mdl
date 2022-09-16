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

#include "lis3mdl.h"  // NOLINT

namespace bfs {

bool Lis3mdl::Begin() {
  if (iface_ == SPI) {
    pinMode(dev_, OUTPUT);
    /* Toggle CS pin to lock in SPI mode */
    digitalWrite(dev_, LOW);
    delay(1);
    digitalWrite(dev_, HIGH);
    delay(1);
  }
  /* Check WHOAMI */
  if (!ReadRegisters(REG_WHO_AM_I_, sizeof(who_am_i_), &who_am_i_)) {
    return false;
  }
  if (who_am_i_ != LIS3MDL_WHOAMI_) {
    return false;
  }
  /* Enable temperature sensor */
  if (!EnableTemp()) {
    return false;
  }
  /* Set range to +/-16GS */
  if (!ConfigRange(RANGE_16GS)) {
    return false;
  }
  /* Set ODR to 155 Hz */
  if (!ConfigOdr(ODR_155HZ)) {
    return false;
  }
  /* Continuous mode */
  if (!EnableContinousMode(true)) {
    return false;
  }
  if (!EnableBdu()) {
    return false;
  }
  while(!Read()) {}
  return true;
}
bool Lis3mdl::ConfigOdr(const Odr odr) {
  if (odr == ODR_155HZ) {
    if (!SetPerfMode(PERF_MODE_ULTRA_HIGH)) {
      return false;
    }
    if (!GetPerfMode(&perf_mode_)) {
      return false;
    }
    if (perf_mode_ != PERF_MODE_ULTRA_HIGH) {
      return false;
    }
    continuous_ = true;
  } else if (odr == ODR_300HZ) {
    if (!SetPerfMode(PERF_MODE_HIGH)) {
      return false;
    }
    if (!GetPerfMode(&perf_mode_)) {
      return false;
    }
    if (perf_mode_ != PERF_MODE_HIGH) {
      return false;
    }
    continuous_ = true;
  } else if (odr == ODR_560HZ) {
    if (!SetPerfMode(PERF_MODE_MEDIUM_POWER)) {
      return false;
    }
    if (!GetPerfMode(&perf_mode_)) {
      return false;
    }
    if (perf_mode_ != PERF_MODE_MEDIUM_POWER) {
      return false;
    }
    continuous_ = true;
  } else if (odr == ODR_1000HZ) {
    if (!SetPerfMode(PERF_MODE_LOW_POWER)) {
      return false;
    }
    if (!GetPerfMode(&perf_mode_)) {
      return false;
    }
    if (perf_mode_ != PERF_MODE_LOW_POWER) {
      return false;
    }
    continuous_ = true;
  } else if (odr == ODR_80HZ) {
    continuous_ = true;
  } else {
    continuous_ = false;
  }
  delay(50);
  if (!SetDataRate(odr)) {
    return false;
  }
  if (!GetDataRate(&odr_)) {
    return false;
  }
  if (odr != odr_) {
    return false;
  }
  if (!EnableContinousMode(continuous_)) {
    return false;
  }
  return true;
}
bool Lis3mdl::ConfigRange(const Range range) {
  if (!SetRange(range)) {
    return false;
  }
  if (!GetRange(&range_)) {
    return false;
  }
  if (range_ != range) {
    return false;
  }
  switch (range) {
    case RANGE_4GS: {
      scale_ = 6842.0f;
      break;
    }
    case RANGE_8GS: {
      scale_ = 3421.0f;
      break;
    }
    case RANGE_12GS: {
      scale_ = 2281.0f;
      break;
    }
    case RANGE_16GS: {
      scale_ = 1711.0f;
      break;
    }
  }
  return true;
}
bool Lis3mdl::Read() {
  if (!ReadRegisters(REG_STATUS_REG_, 1, &stat_)) {
    return false;
  }
  data_updated_ = false;
  if (stat_ & 0x01) {
    data_updated_ = true;
    new_x_data_ = true;
  }
  if (stat_ & 0x02) {
    data_updated_ = true;
    new_y_data_ = true;
  }
  if (stat_ & 0x04) {
    data_updated_ = true;
    new_z_data_ = true;
  }
  if (new_x_data_) {
    if (ReadRegisters(REG_OUT_X_L_, 2, buf_)) {
      x_ = static_cast<int16_t>(buf_[1]) << 8 | buf_[0];
      mag_ut_[0] = static_cast<float>(x_) / scale_ * 100.0f;
    }
  }
  if (new_y_data_) {
    if (ReadRegisters(REG_OUT_Y_L_, 2, buf_)) {
      y_ = static_cast<int16_t>(buf_[1]) << 8 | buf_[0];
      mag_ut_[1] = static_cast<float>(y_) / scale_ * 100.0f;
    }
  }
  if (new_z_data_) {
    if (ReadRegisters(REG_OUT_Z_L_, 2, buf_)) {
      z_ = static_cast<int16_t>(buf_[1]) << 8 | buf_[0];
      mag_ut_[2] = static_cast<float>(z_) / scale_ * 100.0f;
    }
  }
  return data_updated_;
}
bool Lis3mdl::new_x_data() {
  if (new_x_data_) {
    new_x_data_ = false;
    return true;
  }
  return false;
}
bool Lis3mdl::new_y_data() {
  if (new_y_data_) {
    new_y_data_ = false;
    return true;
  }
  return false;
}
bool Lis3mdl::new_z_data() {
  if (new_z_data_) {
    new_z_data_ = false;
    return true;
  }
  return false;
}
bool Lis3mdl::EnableBdu() {
  if (!Write(REG_CTRL_REG5_, 0x01, 1, 6)) {
    return false;
  }
  return true;
}
bool Lis3mdl::EnableContinousMode(bool en) {
  if (en) {
    if (!SetOpMode(OP_MODE_CONTINUOUS)) {
      return false;
    }
    if (!GetOpMode(&op_mode_)) {
      return false;
    }
    if (op_mode_ != OP_MODE_CONTINUOUS) {
      return false;
    }
    return true;
  } else {
    if (!SetOpMode(OP_MODE_SINGLE)) {
      return false;
    }
    if (!GetOpMode(&op_mode_)) {
      return false;
    }
    if (op_mode_ != OP_MODE_SINGLE) {
      return false;
    }
    return true;
  }
}
bool Lis3mdl::EnableTemp() {
  if (!Write(REG_CTRL_REG1_, 0x01, 1, 7)) {
    return false;
  }
  return true;
}
bool Lis3mdl::SetPerfMode(const PerfMode perf_mode) {
  if (!Write(REG_CTRL_REG1_, static_cast<uint8_t>(perf_mode), 2, 5)) {
    return false;
  }
  if (!Write(REG_CTRL_REG4_, static_cast<uint8_t>(perf_mode), 2, 2)) {
    return false;
  }
  return true;
}
bool Lis3mdl::SetOpMode(const OpMode op_mode) {
  if (!Write(REG_CTRL_REG3_, static_cast<uint8_t>(op_mode), 2, 0)) {
    return false;
  }
  return true;
}
bool Lis3mdl::SetDataRate(const Odr data_rate) {
  if (!Write(REG_CTRL_REG1_, static_cast<uint8_t>(data_rate), 4, 1)) {
    return false;
  }
  return true;
}
bool Lis3mdl::SetRange(const Range range) {
  if (!Write(REG_CTRL_REG2_, static_cast<uint8_t>(range), 2, 5)) {
    return false;
  }
  return true;
}
bool Lis3mdl::GetPerfMode(PerfMode * const perf_mode) {
  if (!Read(REG_CTRL_REG1_, 2, 5, &buf_[0])) {
    return false;
  }
  if (!Read(REG_CTRL_REG4_, 2, 2, &buf_[1])) {
    return false;
  }
  if (buf_[0] != buf_[1]) {
    return false;
  }
  (*perf_mode) = static_cast<PerfMode>(buf_[0]);
  return true;
}
bool Lis3mdl::GetOpMode(OpMode * const op_mode) {
  if (!Read(REG_CTRL_REG3_, 2, 0, &buf_[0])) {
    return false;
  }
  (*op_mode) = static_cast<OpMode>(buf_[0]);
  return true;
}
bool Lis3mdl::GetDataRate(Odr * const data_rate) {
  if (!Read(REG_CTRL_REG1_, 4, 1, &buf_[0])) {
    return false;
  }
  (*data_rate) = static_cast<Odr>(buf_[0]);
  return true;
}
bool Lis3mdl::GetRange(Range * const range) {
  if (!Read(REG_CTRL_REG2_, 2, 5, &buf_[0])) {
    return false;
  }
  (*range) = static_cast<Range>(buf_[0]);
  return true;
}
bool Lis3mdl::Read(const uint8_t reg, const uint8_t bits, const uint8_t shift,
                   uint8_t * const data) {
  if (!ReadRegisters(reg, 1, &val_)) {
    return false;
  }
  val_ >>= shift;
  mask_ = (1 << (bits)) - 1;
  *data = val_ & mask_;
  return true;
}
bool Lis3mdl::Write(const uint8_t reg, const uint8_t data, const uint8_t bits,
                    const uint8_t shift) {
  if (!ReadRegisters(reg, 1, &val_)) {
    return false;
  }
  mask_ = (1 << (bits)) - 1;
  data_ = data & mask_;
  mask_ <<= shift;
  val_ &= ~mask_;
  val_ |= data_ << shift;
  return WriteRegister(reg, val_);
}
bool Lis3mdl::WriteRegister(const uint8_t reg, const uint8_t data) {
  uint8_t ret_val;
  if (iface_ == I2C) {
    i2c_->beginTransmission(dev_);
    i2c_->write(reg);
    i2c_->write(data);
    i2c_->endTransmission();
  } else {
    spi_->beginTransaction(SPISettings(SPI_CLOCK_, MSBFIRST, SPI_MODE0));
    #if defined(TEENSYDUINO)
    digitalWriteFast(dev_, LOW);
    #else
    digitalWrite(dev_, LOW);
    #endif
    spi_->transfer(reg);
    spi_->transfer(data);
    #if defined(TEENSYDUINO)
    digitalWriteFast(dev_, HIGH);
    #else
    digitalWrite(dev_, HIGH);
    #endif
    spi_->endTransaction();
  }
  delay(10);
  ReadRegisters(reg, sizeof(ret_val), &ret_val);
  if (data == ret_val) {
    return true;
  } else {
    return false;
  }
}
bool Lis3mdl::ReadRegisters(const uint8_t reg, const uint8_t count,
                            uint8_t * const data) {
  if (iface_ == I2C) {
    i2c_->beginTransmission(dev_);
    i2c_->write(reg);
    i2c_->endTransmission(false);
    bytes_rx_ = i2c_->requestFrom(static_cast<uint8_t>(dev_), count);
    if (bytes_rx_ == count) {
      for (std::size_t i = 0; i < count; i++) {
        data[i] = i2c_->read();
      }
      return true;
    } else {
      return false;
    }
  } else {
    spi_->beginTransaction(SPISettings(SPI_CLOCK_, MSBFIRST, SPI_MODE0));
    #if defined(TEENSYDUINO)
    digitalWriteFast(dev_, LOW);
    #else
    digitalWrite(dev_, LOW);
    #endif
    spi_->transfer(reg | SPI_READ_ | SPI_MS_);
    spi_->transfer(data, count);
    #if defined(TEENSYDUINO)
    digitalWriteFast(dev_, HIGH);
    #else
    digitalWrite(dev_, HIGH);
    #endif
    spi_->endTransaction();
    return true;
  }
}

}  // namespace bfs
