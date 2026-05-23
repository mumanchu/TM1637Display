# TM1637Display
Library for TM1637 1..6 Digit 7-Segment LED Display.

![tm1637-led.jpg](/images/tm1637-led.jpg)

## ** PRELIMINARY **

_Let me know if it doesn't work..._

## Description

The display module can run on 5V or 3.3V. It's best to power it with 3.3V if connected to a 3.3V microcontroller.

Two models of the display are available. One has a colon for a clock display on digit 2. The other has decimal points for a decimal display. 
They do not have both.

## Class Reference
```cpp
class TM1637Display
{
public:
	bool begin(uint clkPin, uint dioPin, uint numberOfDigits = 4);
	void displayOn(bool on);
	void setBrightness(byte brightness);
	void setDotPosition(byte position);
	byte get7SegmentCode(char ch);
	void display7SegmentCode(byte position, byte code7Seg);
	void displayChar(byte position, char ch);
	void displayString(const char* s, byte offset, byte length);
	void displayString(const char* s);
	void clearDisplay();
};
```

## Revision History

| Date  | Revision | Description |
|:---------- |:---------|:----------- |
| 2026.05.23 | 0.0.0	| Preliminary |

<br/>


## Joke of the Week

Matt's Certainty Principal \
_If you measure the mass of a ping pong ball by hitting it with a sledge hammer travelling at the speed of light, it is certain that you won't be able to determine its direction._



