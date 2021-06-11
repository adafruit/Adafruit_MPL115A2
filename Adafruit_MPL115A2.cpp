/*!
 *  @file Adafruit_MPL115A2.cpp
 *
 *  @mainpage Driver for the Adafruit MPL115A2 barometric pressure sensor
 *
 *  @section intro_sec Introduction
 *
 *  Driver for the MPL115A2 barometric pressure sensor
 *
 *  This is a library for the Adafruit MPL115A2 breakout
 *  ----> https://www.adafruit.com/products/992
 *
 *  Adafruit invests time and resources providing this open source code,
 *  please support Adafruit and open-source hardware by purchasing
 *  products from Adafruit!
 *
 *  @section author Author
 *
 *  K.Townsend (Adafruit Industries)
 *
 *  @section license License
 *
 *  BSD (see license.txt)
 *
 *  @section history
 *
 *  v1.0 - First release
 *  v1.1 - Rick Sellens added casts to make bit shifts work below 22.6C
 *       - get both P and T with a single call to getPT
 */

#include "Adafruit_MPL115A2.h"

/*!
 *  @brief  Gets the factory-set coefficients for this particular sensor
 */
void Adafruit_MPL115A2::readCoefficients() {
  int16_t a0coeff;
  int16_t b1coeff;
  int16_t b2coeff;
  int16_t c12coeff;

  uint8_t cmd;
  uint8_t buffer[8];

  cmd = MPL115A2_REGISTER_A0_COEFF_MSB;
  _i2c_dev->write_then_read(&cmd, 1, buffer, 8);

  a0coeff = (((uint16_t)buffer[0] << 8) | buffer[1]);
  b1coeff = (((uint16_t)buffer[2] << 8) | buffer[3]);
  b2coeff = (((uint16_t)buffer[4] << 8) | buffer[5]);
  c12coeff = (((uint16_t)buffer[6] << 8) | buffer[7]) >> 2;

  /*
  Serial.print("A0 = "); Serial.println(a0coeff, HEX);
  Serial.print("B1 = "); Serial.println(b1coeff, HEX);
  Serial.print("B2 = "); Serial.println(b2coeff, HEX);
  Serial.print("C12 = "); Serial.println(c12coeff, HEX);
  */

  _mpl115a2_a0 = (float)a0coeff / 8;
  _mpl115a2_b1 = (float)b1coeff / 8192;
  _mpl115a2_b2 = (float)b2coeff / 16384;
  _mpl115a2_c12 = (float)c12coeff;
  _mpl115a2_c12 /= 4194304.0;

  /*
  Serial.print("a0 = "); Serial.println(_mpl115a2_a0);
  Serial.print("b1 = "); Serial.println(_mpl115a2_b1);
  Serial.print("b2 = "); Serial.println(_mpl115a2_b2);
  Serial.print("c12 = "); Serial.println(_mpl115a2_c12);
  */
}

/*!
 *  @brief  Instantiates a new MPL115A2 class
 */
Adafruit_MPL115A2::Adafruit_MPL115A2() {
  _mpl115a2_a0 = 0.0F;
  _mpl115a2_b1 = 0.0F;
  _mpl115a2_b2 = 0.0F;
  _mpl115a2_c12 = 0.0F;
}

/*!
 *  @brief  Setups the HW (reads coefficients values, etc.)
 *  @return Returns true if the device was found
 */
bool Adafruit_MPL115A2::begin() {
  return begin(MPL115A2_DEFAULT_ADDRESS, &Wire);
}

/*!
 *  @brief  Setups the HW (reads coefficients values, etc.)
 *  @param  *theWire
 *  @return Returns true if the device was found
 */
bool Adafruit_MPL115A2::begin(TwoWire *theWire) {
  return begin(MPL115A2_DEFAULT_ADDRESS, theWire);
}

/*!
 *  @brief  Setups the HW (reads coefficients values, etc.)
 *  @param  addr
 *  @return Returns true if the device was found
 */
bool Adafruit_MPL115A2::begin(uint8_t addr) { return begin(addr, &Wire); }

/*!
 *  @brief  Setups the HW (reads coefficients values, etc.)
 *  @param  addr
 *  @param  *theWire
 *  @return Returns true if the device was found
 */
bool Adafruit_MPL115A2::begin(uint8_t addr, TwoWire *theWire) {
  if (_i2c_dev) {
    delete _i2c_dev;
  }
  _i2c_dev = new Adafruit_I2CDevice(addr, theWire);

  if (!_i2c_dev->begin()) {
    return false;
  }

  // Read factory coefficient values (this only needs to be done once)
  readCoefficients();
  return true;
}

/*!
 *  @brief  Gets the floating-point pressure level in kPa
 *  @return Pressure in kPa
 */
float Adafruit_MPL115A2::getPressure() {
  float pressureComp, centigrade;

  getPT(&pressureComp, &centigrade);
  return pressureComp;
}

/*!
 *  @brief  Gets the floating-point temperature in Centigrade
 *  @return Temperature in Centigrade
 */
float Adafruit_MPL115A2::getTemperature() {
  float pressureComp, centigrade;

  getPT(&pressureComp, &centigrade);
  return centigrade;
}

/*!
 *  @brief  Gets both at once and saves a little time
 *  @param  *P
 *          Pointer to pressure value
 *  @param  *T
 *          Pointer to temperature value
 */
void Adafruit_MPL115A2::getPT(float *P, float *T) {
  uint16_t pressure, temp;
  float pressureComp;

  uint8_t cmd[2] = {MPL115A2_REGISTER_STARTCONVERSION, 0};
  uint8_t buffer[4];

  _i2c_dev->write(cmd, 2);

  // Wait a bit for the conversion to complete (3ms max)
  delay(5);

  cmd[0] = MPL115A2_REGISTER_PRESSURE_MSB;
  _i2c_dev->write_then_read(cmd, 1, buffer, 4);

  pressure = (((uint16_t)buffer[0] << 8) | buffer[1]) >> 6;
  temp = (((uint16_t)buffer[2] << 8) | buffer[3]) >> 6;

  // See datasheet p.6 for evaluation sequence
  pressureComp = _mpl115a2_a0 +
                 (_mpl115a2_b1 + _mpl115a2_c12 * temp) * pressure +
                 _mpl115a2_b2 * temp;

  // Return pressure and temperature as floating point values
  *P = ((65.0F / 1023.0F) * pressureComp) + 50.0F; // kPa
  *T = ((float)temp - 498.0F) / -5.35F + 25.0F;    // C
}
