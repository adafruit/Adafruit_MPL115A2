/*!
 *  @file Adafruit_MPL115A2.h
 */

#if ARDUINO >= 100
#include "Arduino.h"
#else
#include "WProgram.h"
#endif

#include <Wire.h>

#define MPL115A2_DEFAULT_ADDRESS (0x60) /**< I2C address **/

#define MPL115A2_REGISTER_PRESSURE_MSB                                         \
  (0x00) /**< 10-bit Pressure ADC output value MSB **/
#define MPL115A2_REGISTER_PRESSURE_LSB                                         \
  (0x01) /**< 10-bit Pressure ADC output value LSB **/
#define MPL115A2_REGISTER_TEMP_MSB                                             \
  (0x02) /**< 10-bit Temperature ADC output value MSB **/
#define MPL115A2_REGISTER_TEMP_LSB                                             \
  (0x03) /**< 10-bit Temperature ADC output value LSB **/
#define MPL115A2_REGISTER_A0_COEFF_MSB (0x04)  /**< a0 coefficient MSB **/
#define MPL115A2_REGISTER_A0_COEFF_LSB (0x05)  /**< a0 coefficient LSB **/
#define MPL115A2_REGISTER_B1_COEFF_MSB (0x06)  /**< b1 coefficient MSB **/
#define MPL115A2_REGISTER_B1_COEFF_LSB (0x07)  /**< b1 coefficient LSB **/
#define MPL115A2_REGISTER_B2_COEFF_MSB (0x08)  /**< b2 coefficient MSB **/
#define MPL115A2_REGISTER_B2_COEFF_LSB (0x09)  /**< b2 coefficient LSB **/
#define MPL115A2_REGISTER_C12_COEFF_MSB (0x0A) /**< c12 coefficient MSB **/
#define MPL115A2_REGISTER_C12_COEFF_LSB (0x0B) /**< c12 coefficient LSB **/
#define MPL115A2_REGISTER_STARTCONVERSION                                      \
  (0x12) /**< Start Pressure and Temperature Conversion **/

/*!
 *  @brief  Class that stores state and functions for interacting with
 *          MPL115A2 barometric pressure sensor
 */
class Adafruit_MPL115A2 {
public:
  Adafruit_MPL115A2();
  void begin();
  void begin(TwoWire *theWire);
  void begin(uint8_t addr);
  void begin(uint8_t addr, TwoWire *theWire);

  float getPressure();
  float getTemperature();
  void getPT(float *P, float *T);

private:
  TwoWire *_wire;
  uint8_t _i2caddr;

  float _mpl115a2_a0;
  float _mpl115a2_b1;
  float _mpl115a2_b2;
  float _mpl115a2_c12;

  void readCoefficients();
};
