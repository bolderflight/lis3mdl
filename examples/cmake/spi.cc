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

#include "lis3mdl.h"

bfs::Lis3mdl mag(&SPI, 10);

bool status;
unsigned int t1, t2, t3, t4;

void drdy() {

}

int main() {
  Serial.begin(115200);
  while (!Serial) {}
  Serial.println("STARTING TEST");
  SPI.begin();
  bool status = mag.Begin();
  Serial.println(status);
  // attachInterrupt(9, drdy, RISING);
  while (1) {
    t3 = micros();
    status = mag.Read();
    t4 = micros();
    if (status) {
      t1 = micros();
      Serial.print(mag.new_x_data());
      Serial.print("\t");
      Serial.print(mag.new_y_data());
      Serial.print("\t");
      Serial.print(mag.new_z_data());
      Serial.print("\t");
      Serial.print(t1 - t2);
      Serial.print("\t");
      Serial.print(t4 - t3);
      Serial.print("\n");
      t2 = t1;
    }
  }
}