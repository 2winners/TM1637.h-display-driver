
//  Author: avishorp@gmail.com
//
//  This library is free software; you can redistribute it and/or
//  modify it under the terms of the GNU Lesser General Public
//  License as published by the Free Software Foundation; either
//  version 2.1 of the License, or (at your option) any later version.
//
//  This library is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
//  Lesser General Public License for more details.
//
//  You should have received a copy of the GNU Lesser General Public
//  License along with this library; if not, write to the Free Software
//  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
//  Edit by 2winners @mari-caribbean
extern "C" {
  #include <stdlib.h>
  #include <string.h>
  #include <inttypes.h>
}

#include <TM1637Display.h>
#include <Arduino.h>

#define TM1637_I2C_COMM1    0x40
#define TM1637_I2C_COMM2    0xC0
#define TM1637_I2C_COMM3    0x80

//
//      A
//     ---
//  F |   | B
//     -G-
//  E |   | C
//     ---
//      D
bool dbldot;
const uint8_t digitToSegment[] = {
 // XGFEDCBA
  0b00111111,    // 0
  0b00000110,    // 1
  0b01011011,    // 2
  0b01001111,    // 3
  0b01100110,    // 4
  0b01101101,    // 5
  0b01111101,    // 6
  0b00000111,    // 7
  0b01111111,    // 8
  0b01101111,    // 9
  0b01110111,    // A
  0b01111100,    // b
  0b00111001,    // C
  0b01011110,    // d
  0b01111001,    // E
  0b01110001     // F
  };

TM1637Display::TM1637Display(uint8_t pinClk, uint8_t pinDIO){
	// Copy the pin numbers
	m_pinClk = pinClk;
	m_pinDIO = pinDIO;

	// Set the pin direction and default value.
	// Both pins are set as inputs, allowing the pull-up resistors to pull them up
    pinMode(m_pinClk, INPUT);
    pinMode(m_pinDIO,INPUT);
	digitalWrite(m_pinClk, LOW);
	digitalWrite(m_pinDIO, LOW);
}

void TM1637Display::setBrightness(uint8_t brightness, bool on){
	m_brightness = (brightness & 0x7) | (on? 0x08 : 0x00);
}

void TM1637Display::setSegments(const uint8_t segments[], uint8_t length, uint8_t pos){
    // Write COMM1
	start();
	writeByte(TM1637_I2C_COMM1);
	stop();

	// Write COMM2 + first digit address
	start();
	writeByte(TM1637_I2C_COMM2 + (pos & 0x03));

	// Write the data bytes
	for (uint8_t k=0; k < length; k++)
	  writeByte(segments[k]);

	stop();

	// Write COMM3 + brightness
	start();
	writeByte(TM1637_I2C_COMM3 + (m_brightness & 0x0f));
	stop();
}

void TM1637Display::showNumberDec(int num, bool leading_zero, uint8_t length, uint8_t pos){
  showNumberDecEx(num, 0, leading_zero, length, pos);
}

void TM1637Display::showNumberDecEx(int num, uint8_t dots, bool leading_zero,uint8_t length, uint8_t pos){
  uint8_t digits[4];
	const static int divisors[] = { 1, 10, 100, 1000 };
	bool leading = true;

	for(int8_t k = 0; k < 4; k++) {
	    int divisor = divisors[4 - 1 - k];
		int d = num / divisor;
    uint8_t digit = 0;

		if (d == 0) {
		  if (leading_zero || !leading || (k == 3))
		      digit = encodeDigit(d);
	      else
		      digit = 0;
		}
		else {
			digit = encodeDigit(d);
			num -= d * divisor;
			leading = false;
		}
    
    // Add the decimal point/colon to the digit
    digit |= (dots & 0x80); 
    digits[k] = digit;
	}

	setSegments(digits + (4 - length), length, pos);

    dots <<= 1;
	}

void TM1637Display::num(int digit, int number){
	uint8_t data[] = { 0x00, 0x00, 0x00, 0x00 };
    if(dbldot){
		switch (number){ 
    case 0:data[0] = 0b10111111; break;
	case 1:data[0] = 0b10000110; break;
    case 2:data[0] = 0b11011011; break;
    case 3:data[0] = 0b11001111; break;
	case 4:data[0] = 0b11100110; break;
	case 5:data[0] = 0b11101101; break;
	case 6:data[0] = 0b11111101; break;
	case 7:data[0] = 0b10000111; break;
	case 8:data[0] = 0b11111111; break;
	case 9:data[0] = 0b11101111; break;
	case 10:data[0] = 0b10000000; break; // off
	case 11:data[0] = 0b11110111; break; // A 
	case 12:data[0] = 0b11111100; break; // b
	case 13:data[0] = 0b10111001; break; // C
	case 14:data[0] = 0b11011110; break; // d
	case 15:data[0] = 0b11111001; break; // E
	case 16:data[0] = 0b11110001; break; // F
	case 17:data[0] = 0b11111101; break; // G
	case 18:data[0] = 0b11110110; break; // H I=1
	case 19:data[0] = 0b10000111; break; // J
	case 20:data[0] = 0b10111000; break; // L
	case 21:data[0] = 0b11011100; break; // n o = 0
	case 22:data[0] = 0b11110011; break; // P
	case 23:data[0] = 0b11100111; break; // q
	case 24:data[0] = 0b11110111; break; // R
	case 25:data[0] = 0b11111000; break; // t
	case 26:data[0] = 0b10111110; break; // U
	case 27:data[0] = 0b10011100; break; // u
	case 28:data[0] = 0b11011100; break; // 0
	case 30:data[0] = 0b10000001; break; // top -
	case 31:data[0] = 0b11000000; break; // middle -
	case 32:data[0] = 0b10001000; break; // low -
	case 33:data[0] = 0b11000001; break; // = high
	case 34:data[0] = 0b11001000; break; // = low
	case 35:data[0] = 0b11001001; break; // - 3 x
	case 36:data[0] = 0b10100000; break; // | left top
	case 37:data[0] = 0b10000010; break; // | right top
	case 38:data[0] = 0b10010000; break; // | left bottom
	case 39:data[0] = 0b10000100; break; // | right bottom
	case 40:data[0] = 0b10100010; break; // || top
	case 41:data[0] = 0b10010100; break; // || bottom
	case 42:data[0] = 0b10110110; break; // || top and bottom
	case 43:data[0] = 0b11111110; break; // o+||
	}
	}
	else{
	switch (number){
    case 0:data[0] = 0b00111111; break;
	case 1:data[0] = 0b00000110; break;
    case 2:data[0] = 0b01011011; break;
    case 3:data[0] = 0b01001111; break;
	case 4:data[0] = 0b01100110; break;
	case 5:data[0] = 0b01101101; break;
	case 6:data[0] = 0b01111101; break;
	case 7:data[0] = 0b00000111; break;
	case 8:data[0] = 0b01111111; break;
	case 9:data[0] = 0b01101111; break;
	case 10:data[0] = 0b00000000; break; // off
	case 11:data[0] = 0b01110111; break; // A 
	case 12:data[0] = 0b01111100; break; // b
	case 13:data[0] = 0b00111001; break; // C
	case 14:data[0] = 0b01011110; break; // d
	case 15:data[0] = 0b01111001; break; // E
	case 16:data[0] = 0b01110001; break; // F
	case 17:data[0] = 0b01111101; break; // G
	case 18:data[0] = 0b01110110; break; // H I=1
	case 19:data[0] = 0b00000111; break; // J
	case 20:data[0] = 0b00111000; break; // L
	case 21:data[0] = 0b01011100; break; // n o = 0
	case 22:data[0] = 0b01110011; break; // P
	case 23:data[0] = 0b01100111; break; // q
	case 24:data[0] = 0b01110111; break; // R
	case 25:data[0] = 0b01111000; break; // t
	case 26:data[0] = 0b00111110; break; // U
	case 27:data[0] = 0b00011100; break; // u
	case 28:data[0] = 0b01011100; break; // 0
	case 30:data[0] = 0b00000001; break; // top -
	case 31:data[0] = 0b01000000; break; // middle -
	case 32:data[0] = 0b00001000; break; // low -
	case 33:data[0] = 0b01000001; break; // = high
	case 34:data[0] = 0b01001000; break; // = low
	case 35:data[0] = 0b01001001; break; // - 3 x
	case 36:data[0] = 0b00100000; break; // | left top
	case 37:data[0] = 0b00000010; break; // | right top
	case 38:data[0] = 0b00010000; break; // | left bottom
	case 39:data[0] = 0b00000100; break; // | right bottom
	case 40:data[0] = 0b00100010; break; // || top
	case 41:data[0] = 0b00010100; break; // || bottom
	case 42:data[0] = 0b00110110; break; // || top and bottom
	case 43:data[0] = 0b01111110; break; // o+||
	
	
	}
  }
  setSegments(data, 1, digit);
}

void TM1637Display::doubledot(bool DD){
  dbldot = DD;
}

void TM1637Display::clean(){
num(0,10);num(1,10);num(2,10);num(3,10);
}

void TM1637Display::bitDelay(){
	delayMicroseconds(100);
}

void TM1637Display::start(){
  pinMode(m_pinDIO, OUTPUT);
  bitDelay();
}

void TM1637Display::stop(){
	pinMode(m_pinDIO, OUTPUT);
	bitDelay();
	pinMode(m_pinClk, INPUT);
	bitDelay();
	pinMode(m_pinDIO, INPUT);
	bitDelay();
}

bool TM1637Display::writeByte(uint8_t b){
  uint8_t data = b;

  // 8 Data Bits
  for(uint8_t i = 0; i < 8; i++) {
    // CLK low
    pinMode(m_pinClk, OUTPUT);
    bitDelay();

	// Set data bit
    if (data & 0x01)
      pinMode(m_pinDIO, INPUT);
    else
      pinMode(m_pinDIO, OUTPUT);

    bitDelay();

	// CLK high
    pinMode(m_pinClk, INPUT);
    bitDelay();
    data = data >> 1;
  }

  // Wait for acknowledge
  // CLK to zero
  pinMode(m_pinClk, OUTPUT);
  pinMode(m_pinDIO, INPUT);
  bitDelay();

  // CLK to high
  pinMode(m_pinClk, INPUT);
  bitDelay();
  uint8_t ack = digitalRead(m_pinDIO);
  if (ack == 0)
    pinMode(m_pinDIO, OUTPUT);


  bitDelay();
  pinMode(m_pinClk, OUTPUT);
  bitDelay();

  return ack;
}

uint8_t TM1637Display::encodeDigit(uint8_t digit){
	return digitToSegment[digit & 0x0f];
}
