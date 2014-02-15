//#define DEBUG
#include "DebugUtils.h"

#include "Arduino.h"
#include "DaylightSensor.h"
#include <EEPROM.h>

#pragma region Constants

const byte DEFAULT_NIGHT_LEVEL = 15;
const byte DEFAULT_DAY_LEVEL = 80;
const int SAMPLE_TIME = 500;

#pragma endregion

#pragma region Private Event Handlers

void (*_onChangingFunc)();
void (*_onCancelledFunc)();
void (*_onChangeFunc)(boolean isDaytime);
void (*_onSetLevelFunc)();

#pragma endregion

#pragma region Constructor

DaylightSensor::DaylightSensor() 
{
}

#pragma endregion

#pragma region Public Methods

/// <summary>DaylightSensor::Init - Initialize DaylightSensor object.</summary>
void DaylightSensor::Init(int prPin, boolean isDaytime, long delayTime)
{
  _prPin = prPin;
  _isDaytime = isDaytime;
  _dayLevel = DEFAULT_DAY_LEVEL;
  _nightLevel = DEFAULT_NIGHT_LEVEL;
  _delayTime = delayTime;
  
  byte savedDayLevel = EEPROM.read(0);
  byte savedNightLevel = EEPROM.read(1);
  
  if (savedDayLevel > savedNightLevel)
  {
	  if (savedDayLevel != 255)
		_dayLevel = savedDayLevel;
    
	  if (savedNightLevel != 255)
		_nightLevel = savedNightLevel;
  } 

  DEBUG_PRINTLN("DaylightSensor Init: day level=" + String(_dayLevel) + ", night level=" + String(_nightLevel) + ", delay=" + String(_delayTime));
}

/// <summary>DaylightSensor::Reset - Manually reset the daylight sensor to some state.</summary>
void DaylightSensor::Reset(boolean isDaytime)
{
  _isDaytime = isDaytime;
  DEBUG_PRINTLN("After Reset: isDaytime=" + String(_isDaytime));
  _changeStart = 0;
}

/// <summary>DaylightSensor::IsDaytime - Return current daytime state.</summary>
boolean DaylightSensor::IsDaytime()
{
  return _isDaytime;
}

/// <summary>DaylightSensor::SetDayLevel - Save current light level to memory as day level.</summary>
void DaylightSensor::SetDayLevel()
{
  // Save current reading to EEPROM
  _dayLevel = _currentLevel();
  EEPROM.write(0, _dayLevel);
  DEBUG_PRINTLN("Saved new day level: " + String(_dayLevel));
  if (_onSetLevelFunc)
	  _onSetLevelFunc();
}

/// <summary>DaylightSensor::SetNightLevel - Save current light level to memory as night level.</summary>
void DaylightSensor::SetNightLevel()
{
  // Save current reading to EEPROM
  _nightLevel = _currentLevel();
  EEPROM.write(1, _nightLevel);
  DEBUG_PRINTLN("Saved new night level: " + String(_nightLevel));
  if (_onSetLevelFunc)
	  _onSetLevelFunc();
}

/// <summary>DaylightSensor::Update - Event processor for DaylightSensor.</summary>
boolean DaylightSensor::Update()
{
	boolean hasUpdate = true;
	byte currentLevel = _currentLevel();

	if ((_isDaytime && (currentLevel <= _nightLevel)) || (!_isDaytime && (currentLevel >= _dayLevel))) 
	{
		if (_changeStart == 0) // && ((_isDaytime && (_lastLevel > _nightLevel)) || (!_isDaytime && (_lastLevel < _dayLevel))))
		{
			DEBUG_PRINTLN("Daylight state seems to be different");
			// Changing since last check
			if (_onChangingFunc)
			_onChangingFunc();
    
			_changeStart = millis();
		}
		else if (millis() > (_changeStart + _delayTime))
		{
			DEBUG_PRINTLN("Daylight change detected");
			// Change accepted
			_isDaytime = !_isDaytime;
			if (_onChangeFunc)
			_onChangeFunc(_isDaytime);
			_changeStart = 0;
		}
		// else change still pending
	}
	else if (_changeStart != 0 && ((_isDaytime && _lastLevel <= _nightLevel) || (!_isDaytime && _lastLevel >= _dayLevel)))
	{
		DEBUG_PRINTLN("Daylight change never mind...");
		// Change reverted
		if (_onCancelledFunc)
			_onCancelledFunc();
		_changeStart = 0;
	}
	else
		hasUpdate = false;
  
  _lastLevel = currentLevel;

  return hasUpdate;
}

#pragma endregion

#pragma region Private Methods

byte DaylightSensor::_currentLevel() 
{
  return (byte)map(analogRead(_prPin), 0, 1023, 0, 254);
}

#pragma endregion

#pragma region Events

/// <summary>DaylightSensor::OnChanging - Set event for when sensor detects start of change.</summary>
void DaylightSensor::OnChanging(void (*onChanging)())
{
  _onChangingFunc = onChanging;
}

/// <summary>DaylightSensor::OnCancelled - Set event for when detected change is cancelled.</summary>
void DaylightSensor::OnCancelled(void (*onCancelled)())
{
  _onCancelledFunc = onCancelled;
}

/// <summary>DaylightSensor::OnChange - Set event for when change is confirmed.</summary>
void DaylightSensor::OnChange(void (*onChange)(boolean isDaytime))
{
  _onChangeFunc = onChange;
}

/// <summary>DaylightSensor::OnSetLevel - Set event for when day/night light level is saved to memory.</summary>
void DaylightSensor::OnSetLevel(void (*onSetLevel)())
{
  _onSetLevelFunc = onSetLevel;
}

#pragma endregion


