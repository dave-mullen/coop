//#define DEBUG
#include "DebugUtils.h"

#include "Arduino.h"
#include "Flasher.h"

#pragma region Constants

const long FLASHER_PULSE_LENGTH = 1000;
const long FLASHER_PULSE_DELAY = 0;
const long FLASHER_BLINK_LENGTH = 500;
const long FLASHER_BLINK_DELAY = 500;
const long FLASHER_BLINKFAST_LENGTH = 200;
const long FLASHER_BLINKFAST_DELAY = 200;

#pragma endregion

#pragma region Private Event Handlers

void (*_onPulseCompleteFunc)();

#pragma endregion

#pragma region Constructor

Flasher::Flasher() 
{
}

#pragma endregion

#pragma region Public Methods

/// <summary>Flasher::Init - Initialize Flasher object.</summary>
void Flasher::Init(int ledPin)
{
	_ledPin = ledPin;
	_type = None;
	_lastValue = -1;
}

/// <summary>Flasher::SetFlash - Set current flasher to specified one.</summary>
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

/// <summary>Flasher::SetFlashSequence - Set a sequence of flasher types, setting type for number of milliseconds then switching to secondary type.</summary>
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

/// <summary>Flasher::Update - Event processor for Flasher.</summary>
boolean Flasher::Update() 
{
	if (_type == None && _lastValue == 0)
		return false;

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

	return true;
}

/// <summary>Flasher::IsType - Return whether current flasher is specified type.</summary>
boolean Flasher::IsType(enum FlashType type)
{
	return _type == type;
}

#pragma endregion

#pragma region Private Methods

void Flasher::_writeToPin(int value)
{
	bool pulseJustCompleted = value == 0 && _lastValue != 0 && _type == Pulse;

	if (value != _lastValue)
		analogWrite(_ledPin, value);
	_lastValue = value;

	if (pulseJustCompleted && _onPulseCompleteFunc)
		_onPulseCompleteFunc();
}

#pragma endregion

#pragma region Events

/// <summary>Flasher::OnPulseComplete - Set event handler for when each pulse completes.</summary>
void Flasher::OnPulseComplete(void (*onPulseComplete)())
{
  _onPulseCompleteFunc = onPulseComplete;
}

#pragma endregion