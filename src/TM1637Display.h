#pragma once

/////////////////////////////////////////////////////////////////////
// Driver for TM1637 7-Segment LED display with 1..6 digits
// Copyright (C) 2026.05.22, mumanchu & muman.ch
// https://github.com/mumanchu
// https://muman.ch
/*
Q.	There are several TM1637 libraries out there. Why re-invent the wheel?
A.	Some wheels are more efficient than others ;-)

The display module can run on 5V or 3.3V. It's best to power it with 3.3V 
if connected to a 3.3V microcontroller.

Two models of the display are available. One has a colon for a clock 
display on digit 2. The other has decimal points for a decimal display. 
They do not have both.
*/

// These are normally defined in MumanchuDebug.h
#define LOGERROR(s) { Serial.println(s); Serial.flush(); }
#define ASSERT(b) if (!(b)) { LOGERROR("ASSERT failed"); return false; }

#define ASSERT2(b) if (!(b)) { LOGERROR("ASSERT failed"); return; }


class TM1637Display
{
protected:
	uint clkPin;
	uint datPin;
	uint numDigits;
	static const byte charMap[38];
	static const char* charSet;
	byte currentBrightness;
	bool displayIsOn;
	byte dotPosition;

public:
	bool begin(uint clkPin, uint dioPin, uint numberOfDigits = 4);
	void displayOn(bool on);
	void setBrightness(byte brightness);
	void setDotPosition(byte position);
	byte get7SegmentCode(char ch);
	void display7SegmentCode(byte position, byte code7Seg);
	void displayChar(byte position, char ch);
	void displayString(const char* s, byte offset, byte length);
	void displayString(const char* s) { displayString(s, 0, numDigits); }
	void clearDisplay();

protected:
	void writeChars(const char* data, byte startPosition, 
		byte length, bool ascii = true);
	bool writeCommand(byte cmd);
	bool clockOutByte(byte data);
};


// 7-segment codes for decimal, hex and other supported characters
// segments are numbered a..g, x is the decimal point '.'
// bit 76543210
//     xgfedcba
// 
//         a
//        ---
//     f |   | b
//        ---  g
//     e |   | c
//        ---  . x (dp)
//         d
// 
const byte TM1637Display::charMap[38] =
{
	//xgfedcba
	0b00111111,    // 0 [0]
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
	0b01011000,    // c
	0b01011110,    // d
	0b01111001,    // E
	0b01110001,    // F [15]

	// some text characters are supported
	//xgfedcba
	0b00000000,    // space	[16]
	0b01000000,    // -
	0b01110110,    // H 
	0b01110100,    // h
	0b00110000,    // I
	0b00010000,    // i
	0b00001110,    // J
	0b00111000,    // L
	0b00110000,    // l
	0b00111111,    // O
	0b01011100,    // o
	0b01110011,    // P
	0b01100111,    // q
	0b01010000,    // r
	0b01101101,    // S
	0b01111000,    // t
	0b00111110,    // U
	0b00011100,    // u
	0b01101110,    // y
	0b00111001,    // C [35]
	0b01100011,    // ° \xB0 (degrees) [36]

	0b00000000     // NUL [37]
};

// List of supported characters
// offset into this string is the offset into charMap[]
const char* TM1637Display::charSet =
//   0         1         2         3      
//   0123456789012345678901234567890123456
	"0123456789AbcdEF -HhIiJLlOoPqrStUuyC\xB0";
//   |---- hex -----|                    °


// Call this from setup()
bool TM1637Display::begin(uint clockPin, uint dataPin, uint numberOfDigits /*=4*/)
{
	ASSERT(digitalPinToPinName(clockPin) != NC && digitalPinToPinName(dataPin) != NC);
	ASSERT(numberOfDigits >= 1 && numberOfDigits <= 6);

	clkPin = clockPin;
	datPin = dataPin;
	numDigits = numberOfDigits;

	pinMode(clkPin, OUTPUT);
	digitalWrite(clkPin, 1);
	pinMode(datPin, OUTPUT);
	digitalWrite(datPin, 1);

	displayIsOn = true;
	currentBrightness = 0;
	return true;
}

// Turn on/off the display
void TM1637Display::displayOn(bool on)
{
	displayIsOn = on;
	byte cmd = (displayIsOn ? 0x88 : 0x80) + currentBrightness;
	writeCommand(cmd);
}

// Brightness is 0..7
void TM1637Display::setBrightness(byte brightness)
{
	if (brightness > 7)
		brightness = 7;
	currentBrightness = brightness;
	byte cmd = (displayIsOn ? 0x88 : 0x80) + currentBrightness;
	writeCommand(cmd);
}

// Set the decimal point position or show the time separator colon ':' 
// position = 0..4, where 0 = no dot, 1..4 = digit position
// 2 = colon (if present) 
// >>> call this BEFORE displaying the value
void TM1637Display::setDotPosition(byte position)
{
	ASSERT2(position < numDigits);
	dotPosition = position;
}

// Returns the 7-segment value of an ASCII character
// or 0 if the character is not supported or is 'space'
byte TM1637Display::get7SegmentCode(char ch)
{
	char* p = strchr(charSet, ch);
	return p ? charMap[p - charSet] : 0;
}

// Display Methods

void TM1637Display::display7SegmentCode(byte position, byte code7Seg)
{
	ASSERT2(position < numDigits);
	char s[2] = { code7Seg, '\0' };
	writeChars(s, position, 1, false);
}

void TM1637Display::displayChar(byte position, char ch)
{
	ASSERT2(position < numDigits);
	char s[2] = { ch, '\0' };
	writeChars(s, position, 1);
}

void TM1637Display::displayString(const char* s, byte position, byte length)
{
	ASSERT2(position < numDigits);
	writeChars(s, position, length);
}

void TM1637Display::clearDisplay()
{
	dotPosition = 0;
	char spaces[numDigits];
	memset(spaces, ' ', numDigits);
	writeChars(spaces, 0, numDigits);
}

// Internal Methods

void TM1637Display::writeChars(const char* data, 
	byte startPosition, byte length, bool ascii /*=true*/)
{
	ASSERT2(startPosition < numDigits && (startPosition + length) <= numDigits);

	// auto increment address command
	writeCommand(0x40);

	// start message
	digitalWrite(datPin, 0);
	digitalWrite(clkPin, 0);

	// start position
	clockOutByte(0xC0 + startPosition);

	// send each display character as 7-segment code
	for (int i = startPosition; i < startPosition + length; ++i) {
		byte b = *data;
		if (b != '\0') {
			if (ascii)
				b = get7SegmentCode(b);
			++data;
		}
		// '.' or ':' position
		if (dotPosition && i == dotPosition - 1)
			b |= 0x80;
		clockOutByte(b);
	}

	// end message
	digitalWrite(clkPin, 1);
	digitalWrite(datPin, 1);
}

bool TM1637Display::writeCommand(byte cmd)
{
	digitalWrite(datPin, 0);
	digitalWrite(clkPin, 0);
	bool ack = clockOutByte(cmd);
	digitalWrite(clkPin, 1);
	digitalWrite(datPin, 1);
	return ack;
}

bool TM1637Display::clockOutByte(byte data)
{
	// clock out the byte, LS bit first
	for (int i = 0; i < 8; ++i) {
		digitalWrite(clkPin, 0);
		digitalWrite(datPin, data & 1);
		digitalWrite(clkPin, 1);
		data >>= 1;
	}

	// get ack
	pinMode(datPin, INPUT);
	// falling edge of 8th clock
	digitalWrite(clkPin, 0);

	// wait for ack for up to 100us
	ulong t1 = micros();
	bool ack = false;
	do {
		if (digitalRead(datPin) == 0) {
			ack = true;
			break;
		}
	} while ((micros() - t1) <= 100);

	digitalWrite(clkPin, 1);
	pinMode(datPin, OUTPUT);
	digitalWrite(clkPin, 0);

	return ack;
}

