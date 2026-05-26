#pragma once

/////////////////////////////////////////////////////////////////////
// Driver for TM1637 7-Segment LED display with 2..6 digits
// Copyright (C) 2026.05.24, mumanchu & muman.ch
// https://github.com/mumanchu
// https://muman.ch
/*
Q.	There are several TM1637 libraries out there. Why re-invent the wheel?
A.	New wheels give a smoother ride.

Most methods return bool: true = success, false = failed (ASSERT or no 'ack')

The display module can run on 5V or 3.3V. If connected to a 3.3V MCU it is 
best to power it from 3.3V. Or use a logic-level converter because the 
board has 10K pullup resistors to 5V which may eventually damage the MCU's
3.3V pins if they are not 5V-tolerant.

Two models of the display are available. One has a colon for a clock 
display on digit 2. The other has decimal points for a decimal display. 
They do not have both.

DATA SHEET
Take care, it's written in Chinglish
https://muman.ch/pub/tm1637_datasheet.pdf
*/

// These are normally defined in MumanchuDebug.h
#ifndef ASSERT
#define LOGERROR(s) { Serial.println(s); Serial.flush(); }
#define ASSERT(b) if (!(b)) { LOGERROR("ASSERT failed"); return false; }
#endif


class TM1637Display
{
protected:
	uint pinClk;
	uint pinDio;
	uint numDigits;
	static const byte charMap[38];
	static const char* charSet;
	byte currentBrightness;
	bool displayIsOn;
	byte dotPosition;

	// This delay may be needed for some [cheap] modules.
	// The 100pF capacitors on DIO and CLK can be too big, 
	// which slows the signals. Some TM1637 chips may be 
	// clones or out-of-spec chips.
	const uint delayInMicroseconds = 0;

public:
	bool begin(uint clkPin, uint dioPin, uint numberOfDigits = 4);
	bool displayOn(bool on);
	bool setBrightness(byte brightness);
	bool clearDisplay();
	void setDotPosition(byte position);
	byte get7SegmentCode(char ch);
	bool display7SegmentCode(byte position, byte code7Seg);
	bool displayChar(byte position, char ch);
	bool displayString(const char* s);
	bool displayString(const char* s, byte offset, byte length);

protected:
	bool writeChars(const char* data, byte startPosition, 
		byte length, bool ascii = true);
	bool writeCommand(byte cmd);
	bool writeByte(byte b);
	void digitalWriteEx(uint pin, uint b);
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
	0b01100011,    // \xb0 (degrees) [36]

	0b00000000     // NUL [37]
};

// Supported characters
// offset into this string is the offset into charMap[]
const char* TM1637Display::charSet =
//   0         1         2         3      
//   0123456789012345678901234567890123456
	"0123456789AbcdEF -HhIiJLlOoPqrStUuyC\xb0";
//   ----- hex ------


// Call this from setup()
// it returns false on invalid pin number 
// or if the chip does not respond (no 'ack')
bool TM1637Display::begin(uint clockPin, uint dataPin, uint numberOfDigits /*=4*/)
{
	ASSERT(digitalPinToPinName(clockPin) != NC && digitalPinToPinName(dataPin) != NC);
	ASSERT(numberOfDigits >= 1 && numberOfDigits <= 6);

	pinClk = clockPin;;
	pinDio = dataPin;
	numDigits = numberOfDigits;
	displayIsOn = true;
	currentBrightness = 0;

	pinMode(pinClk, OUTPUT);
	pinMode(pinDio, OUTPUT);
	digitalWrite(pinClk, 1);
	digitalWrite(pinDio, 1);

	// send auto increment address command
	// the chip remains in this mode
	if (!writeCommand(0x40))
		return false;
	
	return clearDisplay();
}

// Turn on/off the display
bool TM1637Display::displayOn(bool on)
{
	displayIsOn = on;
	byte cmd = (displayIsOn ? 0x88 : 0x80) + currentBrightness;
	return writeCommand(cmd);
}

// Brightness is 0..7
bool TM1637Display::setBrightness(byte brightness)
{
	if (brightness > 7)
		brightness = 7;
	currentBrightness = brightness;
	byte cmd = (displayIsOn ? 0x88 : 0x80) + currentBrightness;
	return writeCommand(cmd);
}

bool TM1637Display::clearDisplay()
{
	dotPosition = 0;
	char spaces[numDigits];
	memset(spaces, ' ', numDigits);
	return writeChars(spaces, 0, numDigits);
}

// Set the decimal point position or show the time separator colon ':' 
// position = 0..4, where 0 = no dot, 1..4 = digit position
// 2 = colon (if present) 
// >>> CALL THIS BEFORE DISPLAYING THE VALUE <<<
inline void TM1637Display::setDotPosition(byte position)
{
	if (position < numDigits);
		dotPosition = position;
}

// Returns the 7-segment value of an ASCII character
// or 0 if the character is not supported or is 'space'
inline byte TM1637Display::get7SegmentCode(char ch)
{
	char* p = strchr(charSet, ch);
	return p ? charMap[p - charSet] : 0;
}

// Display Methods

inline bool TM1637Display::display7SegmentCode(byte position, byte code7Seg)
{
	return writeChars((char*)&code7Seg, position, 1, false);
}

inline bool TM1637Display::displayChar(byte position, char ch)
{
	return writeChars(&ch, position, 1);
}

inline bool TM1637Display::displayString(const char* s) 
{ 
	return writeChars(s, 0, numDigits);
}

inline bool TM1637Display::displayString(const char* s, byte position, byte length)
{
	return writeChars(s, position, length);
}

// Internal Methods

bool TM1637Display::writeChars(const char* data,
	byte startPosition, byte length, bool ascii /*=true*/)
{
	ASSERT(startPosition < numDigits && (startPosition + length) <= numDigits);

	// start message
	digitalWriteEx(pinDio, 0);

	// send start position
	writeByte(0xc0 + (startPosition & 0x03));

	// send each display character as 7-segment code
	byte ack = true;
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
		ack &= writeByte(b);
	}

	// end message
	digitalWriteEx(pinClk, 1);
	digitalWriteEx(pinDio, 1);

	return ack;
}

bool TM1637Display::writeCommand(byte cmd)
{
	// start message
	digitalWriteEx(pinDio, 0);

	// send command
	bool ack = writeByte(cmd);

	// end message
	digitalWriteEx(pinClk, 1);
	digitalWriteEx(pinDio, 1);

	return ack;
}

bool TM1637Display::writeByte(byte b)
{
	// clock out the byte, LS bit first
	for (uint i = 0; i < 8; i++) {
		digitalWriteEx(pinClk, 0);
		digitalWriteEx(pinDio, b & 1);
		digitalWriteEx(pinClk, 1);
		b >>= 1;
	}

	// after the falling edge of the 8th clock
	// the TM1637 will drive the line low to indicate ack
	digitalWriteEx(pinClk, 0);
	// change to input so we can read it
	pinMode(pinDio, INPUT);
	digitalWriteEx(pinClk, 1);

	// wait for ack for up to 100us
	// normally takes 3 or 4 us
	ulong t1 = micros();
	bool ack = false;
	do {
		if (digitalRead(pinDio) == 0) {
			ack = true;
			break;
		}
	} while ((micros() - t1) <= 100);

	// data pin back to output
	pinMode(pinDio, OUTPUT);

	// end message
	digitalWriteEx(pinDio, 0);
	digitalWriteEx(pinClk, 0);

	return ack;
}

// digitalWrite() with a delay of a few microseconds
// note that the call itself takes a few microseconds, 
// which is usually enough
void TM1637Display::digitalWriteEx(uint pin, uint b)
{
	digitalWrite(pin, b);
	delayMicroseconds(delayInMicroseconds);
}

