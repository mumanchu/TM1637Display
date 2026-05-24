# TM1637Display
Library for TM1637 1..6 Digit 7-Segment LED Display.

![tm1637-led.jpg](/images/tm1637-led.jpg)

## ** PRELIMINARY **

_Let me know if it doesn't work..._

## Description

The display module can run on 5V or 3.3V. If connected to a 3.3V microcontroller it is best to power it from 3.3V. Or use a logic-level converter because the board has 10K pullup resistors to 5V which may eventually damage the microcontroller's 3.3V pins if they are not 5V-tolerant.

Two models of the display are available. One has a colon for a clock display on digit 2. The other has decimal points for a decimal display. 
They do not have both.

## Class Reference

Most methods return `bool`: `true` = success, `false` = failed (ASSERT or no 'ack').

`displayString()` does not handle decimal points, they are shown as blanks. Instead, use `setDotPosition()`.

For formatting numbers for display, take a look at `sprintf()`.

```cpp
class TM1637Display
{
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
};
```

## Data Sheet

Take care, it's written in Chinglish \
https://cdn.velleman.eu/downloads/29/infosheets/tm1637_datasheet.pdf

## Revision History

| Date  | Revision | Description |
|:---------- |:---------|:----------- |
| 2026.05.23 | 0.0.0	| Preliminary |

<br/>


## Joke of the Week

**Matt's Certainty Principal** \
_If you measure the mass of a ping pong ball by hitting it with a sledge hammer travelling at close to the speed of light, it is certain that you won't be able to determine its direction._ 

(ref. CERN's Large Hardon Collider)

