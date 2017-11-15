//#define DEBUG
#include "DebugUtils.h"

#include "Arduino.h"
#include "Door.h"

#pragma region Constants

const int DOOR_OPENED_STATE = LOW; // Note: Switch mistakenly wired as NC so will go low when open
const int DOOR_CLOSED_STATE = HIGH; 

#pragma endregion

#pragma region Private Event Handlers

void (*_onMaxRunFunc)();
void (*_onOpenedFunc)();
void (*_onClosedFunc)();
void (*_onDoorStartFunc)();

#pragma endregion

#pragma region Constructor

Door::Door() 
{
  _doorState = Idle;
}

#pragma endregion

#pragma region Public Methods

/// <summary>Door::Init - Initialize Door object.</summary>
void Door::Init(int doorOpenedSwitchPin, int doorClosedSwitchPin, int motorStbyPin, int motorPwmPin, int motorIn1Pin, int motorIn2Pin, byte doorSpeed, int maxOpenTime, int maxCloseTime)
{
  _doorOpenedSwitchPin = doorOpenedSwitchPin;
  _doorClosedSwitchPin = doorClosedSwitchPin;  
  _motorStbyPin = motorStbyPin;
  _motorPwmPin = motorPwmPin;
  _motorIn1Pin = motorIn1Pin;
  _motorIn2Pin = motorIn2Pin;
  _doorSpeed = doorSpeed;
  _maxOpenTime = maxOpenTime;
  _maxCloseTime = maxCloseTime;
  
  // Init pins
  pinMode(_doorOpenedSwitchPin, INPUT);
  pinMode(_doorClosedSwitchPin, INPUT);

  pinMode(_motorStbyPin, OUTPUT);
  pinMode(_motorPwmPin, OUTPUT);
  pinMode(_motorIn1Pin, OUTPUT);
  pinMode(_motorIn2Pin, OUTPUT);

  // Initialize door state, if possible
  if (digitalRead(_doorOpenedSwitchPin) == DOOR_OPENED_STATE) 
    _doorState = Opened;
  else if (digitalRead(_doorClosedSwitchPin) == DOOR_CLOSED_STATE)
    _doorState = Closed;

  DEBUG_PRINTLN("Door Init: _doorState=" + String(_doorState));
}

/// <summary>Door::Update - Event processor for Door.</summary>
boolean Door::Update()
{
	boolean hasUpdate = true;

  if (_doorState == Opening || _doorState == Closing) 
  {
    if (_doorState != _lastDoorState)
      _doorRunStart = millis();

    if (_doorState == Opening && digitalRead(_doorOpenedSwitchPin) == DOOR_OPENED_STATE) 
    {
      DEBUG_PRINTLN("Door detected opened.  Stopping...");
      
      Stop();
      
      if (_onOpenedFunc)
        _onOpenedFunc();
    }
    else if (_doorState == Closing && digitalRead(_doorClosedSwitchPin) == DOOR_CLOSED_STATE) 
    {
      DEBUG_PRINTLN("Door detected closed.  Stopping...");
      
      Stop();
      
      if (_onClosedFunc)
        _onClosedFunc();
    }
    else 
    {
      if ((millis() - _doorRunStart) > (_doorState == Opening ? _maxOpenTime : _maxCloseTime)) 
      {
        DEBUG_PRINTLN("Door timed out while " + String(_doorState) + ".  Stopping...");

        // door has hit max running time
        Stop();

        if (_onMaxRunFunc)
          _onMaxRunFunc();
      }
    }
  }
  else
  {
    _doorRunStart = 0;
	hasUpdate = false;
  }

  _lastDoorState = _doorState;

  return hasUpdate;
}

/// <summary>Door::Open - Start door opening.</summary>
void Door::Open() 
{
  _openClose(true);
}

/// <summary>Door::Close - Start door closing.</summary>
void Door::Close() 
{
  _openClose(false);
}

/// <summary>Door::Toggle - Toggle between opening, idle, and closing.</summary>
void Door::Toggle()
{
	if (_doorState == Opening || _doorState == Closing)
		Stop();
	else
		_openClose(_doorState == Opened ? false : true);
}

/// <summary>Door::Stop - Stop opening or closing door.</summary>
void Door::Stop() 
{
  digitalWrite(_motorStbyPin, LOW);

  switch (_doorState) {
    case Opening:
      _doorState = Opened;
      break;
    case Closing:
      _doorState = Closed;
      break;
    default:
      _doorState = Idle;
      break;
  }
}

/// <summary>Door::IsOpened - Return true if door is open.</summary>
boolean Door::IsOpened()
{
  return _doorState == Opened;
}

/// <summary>Door::IsClosed - Return true if door is closed.</summary>
boolean Door::IsClosed()
{
  return _doorState == Closed;
}

#pragma endregion

#pragma region Private Methods

void Door::_openClose(boolean openIt) 
{
  DEBUG_PRINTLN("Door " + String(openIt ? "opening... " : "closing..."));

  _doorState = openIt ? Opening : Closing;

  // Stop motor if running currently
  analogWrite(_motorPwmPin, 0);

  digitalWrite(_motorStbyPin, HIGH);

  digitalWrite(_motorIn1Pin, openIt ? HIGH : LOW);
  digitalWrite(_motorIn2Pin, openIt ? LOW : HIGH);
  analogWrite(_motorPwmPin, _doorSpeed);

  if (_onDoorStartFunc)
    _onDoorStartFunc();
}

#pragma endregion

#pragma region Events

/// <summary>Door::OnMaxRun - Set event handler for when door has run it's max amount of allowed time.</summary>
void Door::OnMaxRun(void (*onMaxRun)())
{
  _onMaxRunFunc = onMaxRun;
}

/// <summary>Door::OnOpened - Set event handler for when door is fully opened.</summary>
void Door::OnOpened(void (*onOpened)())
{
  _onOpenedFunc = onOpened;
}

/// <summary>Door::OnClosed - Set event handler for when door is fully closed.</summary>
void Door::OnClosed(void (*onClosed)())
{
  _onClosedFunc = onClosed;
}

/// <summary>Door::OnDoorStart - Set event handler for when door starts to open or close.</summary>
void Door::OnDoorStart(void (*onDoorStart)())
{
  _onDoorStartFunc = onDoorStart;
}

#pragma endregion


