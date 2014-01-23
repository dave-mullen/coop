#define DEBUG
#include "DebugUtils.h"

#include "Arduino.h"
#include "DaylightSensor.h"
#include <EEPROM.h>

const byte DEFAULT_NIGHT_LEVEL = 15;
const byte DEFAULT_DAY_LEVEL = 80;
const int SAMPLE_TIME = 500;

void (*_onChangingFunc)();
void (*_onCancelledFunc)();
void (*_onChangeFunc)(boolean isDaytime);
void (*_onSetLevelFunc)();

DaylightSensor::DaylightSensor() 
{
}

void DaylightSensor::Init(int prPin, boolean isDaytime, long delayTime)
{
  _prPin = prPin;
  _isDaytime = isDaytime;
  _dayLevel = DEFAULT_DAY_LEVEL;
  _nightLevel = DEFAULT_NIGHT_LEVEL;
  _delayTime = delayTime;
  
  byte savedDayLevel = EEPROM.read(0);
  byte savedNightLevel = EEPROM.read(1);
  
  if (savedDayLevel != 255)
    _dayLevel = savedDayLevel;
    
  if (savedNightLevel != 255)
    _nightLevel = savedNightLevel;
    
  DEBUG_PRINT("DaylightSensor Init: day level=" + String(_dayLevel) + ", night level=" + String(_nightLevel) + ", delay=" + String(_delayTime));
}

void DaylightSensor::Reset(boolean isDaytime)
{
  _isDaytime = isDaytime;
  DEBUG_PRINT("After Reset: isDaytime=" + String(_isDaytime));
  _changeStart = 0;
}

boolean DaylightSensor::IsDaytime()
{
  return _isDaytime;
}

void DaylightSensor::SetDayLevel()
{
  // Save current reading to EEPROM
  _dayLevel = _currentLevel();
  EEPROM.write(0, _dayLevel);
  DEBUG_PRINT("Saved new day level: " + String(_dayLevel));
  if (_onSetLevelFunc)
	  _onSetLevelFunc();
}

void DaylightSensor::SetNightLevel()
{
  // Save current reading to EEPROM
  _nightLevel = _currentLevel();
  EEPROM.write(1, _nightLevel);
  DEBUG_PRINT("Saved new night level: " + String(_nightLevel));
  if (_onSetLevelFunc)
	  _onSetLevelFunc();
}

void DaylightSensor::Update()
{
  if (!_checkSampleTime())
    return;

  byte currentLevel = _currentLevel();
  
  if ((_isDaytime && (currentLevel <= _nightLevel)) || (!_isDaytime && (currentLevel >= _dayLevel))) 
  {
    if (_changeStart == 0) // && ((_isDaytime && (_lastLevel > _nightLevel)) || (!_isDaytime && (_lastLevel < _dayLevel))))
    {
      // Changing since last check
      if (_onChangingFunc)
        _onChangingFunc();
    
      _changeStart = millis();
    }
    else if (millis() > (_changeStart + _delayTime))
    {
      // Change accepted
      _isDaytime = !_isDaytime;
      if (_onChangeFunc)
        _onChangeFunc(_isDaytime);
      _changeStart = 0;
    }
  }
  else if (_changeStart != 0 && ((_isDaytime && _lastLevel <= _nightLevel) || (!_isDaytime && _lastLevel >= _dayLevel)))
  {
    // Change reverted
    if (_onCancelledFunc)
      _onCancelledFunc();
    _changeStart = 0;
  }  
  
  _lastLevel = currentLevel;
}

byte DaylightSensor::_currentLevel() 
{
  return (byte)map(analogRead(_prPin), 0, 1023, 0, 254);
}

boolean DaylightSensor::_checkSampleTime()
{
  static long lastMillis = millis();
  long currentMillis = millis();

  // Handle overflow
  if (currentMillis < lastMillis)
    lastMillis = currentMillis;
    
  if (currentMillis < (lastMillis + SAMPLE_TIME))
    return false;
    
  lastMillis = currentMillis;

  return true;
}

void DaylightSensor::OnChanging(void (*onChanging)())
{
  _onChangingFunc = onChanging;
}

void DaylightSensor::OnCancelled(void (*onCancelled)())
{
  _onCancelledFunc = onCancelled;
}

void DaylightSensor::OnChange(void (*onChange)(boolean isDaytime))
{
  _onChangeFunc = onChange;
}

void DaylightSensor::OnSetLevel(void (*onSetLevel)())
{
  _onSetLevelFunc = onSetLevel;
}




