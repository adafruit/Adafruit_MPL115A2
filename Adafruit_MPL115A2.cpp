/**************************************************************************/
/*! 
    @file     Adafruit_MPL115A2.cpp
    @author   K.Townsend (Adafruit Industries)
	@license  BSD (see license.txt)
	
	Driver for the MPL115A2 barometric pressure sensor

	This is a library for the Adafruit MPL115A2 breakout
	----> https://www.adafruit.com/products/???
		
	Adafruit invests time and resources providing this open source code, 
	please support Adafruit and open-source hardware by purchasing 
	products from Adafruit!

	@section  HISTORY

    v1.0 - First release
*/
/**************************************************************************/
#if ARDUINO >= 100
 #include "Arduino.h"
#else
 #include "WProgram.h"
#endif

#include <Wire.h>

#include "Adafruit_MPL115A2.h"

/**************************************************************************/
/*! 
    @brief  Gets the factory-set coefficients for this particular sensor
*/
/**************************************************************************/
void Adafruit_MPL115A2::readCoefficients() {
  int16_t a0coeff;
  int16_t b1coeff;
  int16_t b2coeff;
  int16_t c12coeff;

  Wire.beginTransmission(MPL115A2_ADDRESS);
  #if ARDUINO >= 100
    Wire.write((uint8_t)MPL115A2_REGISTER_A0_COEFF_MSB);
  #else
    Wire.send((uint8_t)MPL115A2_REGISTER_A0_COEFF_MSB);
  #endif
  Wire.endTransmission();  

  Wire.requestFrom(MPL115A2_ADDRESS, 8);  
  #if ARDUINO >= 100
    a0coeff = ((Wire.read() << 8) | Wire.read());
    b1coeff = ((Wire.read() << 8) | Wire.read());
    b2coeff = ((Wire.read() << 8) | Wire.read());
    c12coeff = (((Wire.read() << 8) | Wire.read())) >> 2;
  #else
    a0coeff = ((Wire.receive() << 8) | Wire.receive());
    b1coeff = ((Wire.receive() << 8) | Wire.receive());
    b2coeff = ((Wire.receive() << 8) | Wire.receive());
    c12coeff = (((Wire.receive() << 8) | Wire.receive())) >> 2;
  #endif

  _mpl115a2_a0 = (float)a0coeff / 8;
  _mpl115a2_b1 = (float)b1coeff / 8192;
  _mpl115a2_b2 = (float)b2coeff / 16384;
  _mpl115a2_c12 = (float)c12coeff / 4194304;
}

/**************************************************************************/
/*! 
    @brief  Instantiates a new MPL115A2 class
*/
/**************************************************************************/
Adafruit_MPL115A2::Adafruit_MPL115A2() {
  _mpl115a2_a0 = 0.0F;
  _mpl115a2_b1 = 0.0F;
  _mpl115a2_b2 = 0.0F;
  _mpl115a2_c12 = 0.0F;
}

/**************************************************************************/
/*! 
    @brief  Setups the HW (reads coefficients values, etc.)
*/
/**************************************************************************/
void Adafruit_MPL115A2::begin() {
  Wire.begin();    
  // Read factory coefficient values (this only needs to be done once)
  readCoefficients();
}

/**************************************************************************/
/*! 
    @brief  Gets the floating-point pressure level in kPa
*/
/**************************************************************************/
float Adafruit_MPL115A2::getPressure() {
  uint16_t 	pressure, temp;
  float     pressureComp;

  // Get raw pressure and temperature settings
  Wire.beginTransmission(MPL115A2_ADDRESS);
  #if ARDUINO >= 100
    Wire.write((uint8_t)MPL115A2_REGISTER_STARTCONVERSION);
    Wire.write((uint8_t)0x00);
  #else
    Wire.send((uint8_t)MPL115A2_REGISTER_STARTCONVERSION);
    Wire.send((uint8_t)0x00);
  #endif
  Wire.endTransmission();

  // Wait a bit for the conversion to complete (3ms max)
  delay(5);

  Wire.beginTransmission(MPL115A2_ADDRESS);
  #if ARDUINO >= 100
    Wire.write((uint8_t)MPL115A2_REGISTER_PRESSURE_MSB);  // Register
  #else
    Wire.send((uint8_t)MPL115A2_REGISTER_A0_COEFF_MSB);   // Register
  #endif
  Wire.endTransmission();  

  Wire.requestFrom(MPL115A2_ADDRESS, 4);
  #if ARDUINO >= 100
    pressure = ((Wire.read() << 8) | Wire.read()) >> 6;
    temp = ((Wire.read() << 8) | Wire.read()) >> 6;
  #else
    pressure = ((Wire.receive() << 8) | Wire.receive()) >> 6;
    temp = ((Wire.receive() << 8) | Wire.receive()) >> 6;
  #endif

  // See datasheet p.6 for evaluation sequence
  pressureComp = _mpl115a2_a0 + (_mpl115a2_b1 + _mpl115a2_c12 * temp ) * pressure + _mpl115a2_b2 * temp;

  // Return pressure as floating point value
  return ((65.0F / 1023.0F)*(float)pressureComp) + 50;
}


