//#define DEBUG
#include "DebugUtils.h"

#include "Arduino.h"
#include "Flasher.h"

const long FLASHER_PULSE_LENGTH = 1000;
const long FLASHER_PULSE_DELAY = 5000;
const long FLASHER_BLINK_LENGTH = 500;
const long FLASHER_BLINK_DELAY = 500;
const long FLASHER_BLINKFAST_LENGTH = 200;
const long FLASHER_BLINKFAST_DELAY = 200;

Flasher::Flasher() 
{
}

void Flasher::Init(int ledPin)
{
	_ledPin = ledPin;
	_type = None;
	_lastValue = -1;
}

void Flasher::SetFlash(enum FlashType type)
{
	if (type == Revert)
	{
		if (_saveType != Revert)
			_type = _saveType;
	}
	else 
	{
		_saveType = _type;
		_type = type;
	}
	_flashStartTime = millis();
	_flashStopTime = 0;
}

void Flasher::SetFlashSequence(enum FlashType type, int time, enum FlashType afterType)
{
	_saveType = _type;
	_flashStartTime = millis();
	if (afterType == Revert)
		_afterType = _type;
	else
		_afterType = afterType;
	_type = type;
	_flashStopTime = millis() + time;
}

void Flasher::Update() 
{
	long currentMillis = millis();
	//_timeAdjust = 0;

	// Handle overflow
	if (_flashStartTime > currentMillis)
			_flashStartTime = currentMillis;

	if (_flashStopTime != 0 && currentMillis >= _flashStopTime)
	{
		_type = _afterType;
		_flashStartTime = currentMillis;
	}

	switch (_type) {
	case Pulse: 
	{
		// Convert timing to radians value
		float pulseRad = map((currentMillis - _flashStartTime) % (FLASHER_PULSE_LENGTH + FLASHER_PULSE_DELAY), 0, FLASHER_PULSE_LENGTH, 0, 62832) / 10000.0;

		// Convert sin to value 0 to 255
		if (pulseRad <= 6.2832)
		{
			int value = sin(4.712385 + pulseRad) * 127.5 + 127.5;
			_writeToPin(value);
		}
		break;
	}
	case Solid:
		_flashStartTime = 0;
		_writeToPin(255);
		break;
	case Blink:
		if ((currentMillis - _flashStartTime) % (FLASHER_BLINK_LENGTH + FLASHER_BLINK_DELAY) < FLASHER_BLINK_LENGTH)
			_writeToPin(255);
		else
			_writeToPin(0);
		break;
	case BlinkFast:
		if ((currentMillis - _flashStartTime) % (FLASHER_BLINKFAST_LENGTH + FLASHER_BLINKFAST_DELAY) < FLASHER_BLINKFAST_LENGTH)
			_writeToPin(255);
		else
			_writeToPin(0);
		break;
	default:
		_flashStartTime = 0;
		_writeToPin(0);
		break;
	}
}

void Flasher::_writeToPin(int value)
{
	if (value != _lastValue)
		analogWrite(_ledPin, value);
	_lastValue = value;
}

boolean Flasher::IsIdle()
{
	return (_type == None || (_type == Pulse && _lastValue == 0));
}

