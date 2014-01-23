//#define DEBUG
#include "DebugUtils.h"

#include "Arduino.h"
#include "Door.h"

const int DOOR_OPENED_STATE = HIGH;
const int DOOR_CLOSED_STATE = LOW; // Note: Switch mistakenly wired as NC so will go low when closed

void (*_onMaxRunFunc)();
void (*_onOpenedFunc)();
void (*_onClosedFunc)();
void (*_onDoorStartFunc)();

Door::Door() 
{
  _doorState = Idle;
}

void Door::Init(int doorOpenedSwitchPin, int doorClosedSwitchPin, int motorStbyPin, int motorPwmPin, int motorIn1Pin, int motorIn2Pin, byte doorSpeed, int maxRunTime)
{
  _doorOpenedSwitchPin = doorOpenedSwitchPin;
  _doorClosedSwitchPin = doorClosedSwitchPin;  
  _motorStbyPin = motorStbyPin;
  _motorPwmPin = motorPwmPin;
  _motorIn1Pin = motorIn1Pin;
  _motorIn2Pin = motorIn2Pin;
  _doorSpeed = doorSpeed;
  _maxRunTime = maxRunTime;
  
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
}

void Door::Update()
{
	//DEBUG_PRINT(String(digitalRead(_doorOpenedSwitchPin)) + " " + String(digitalRead(_doorClosedSwitchPin)));

  if (_doorState == Opening || _doorState == Closing) 
  {
    if (_doorState != _lastDoorState)
      _doorRunStart = millis();

    if (_doorState == Opening && digitalRead(_doorOpenedSwitchPin) == DOOR_OPENED_STATE) 
    {
      DEBUG_PRINT("Door detected opened.  Stopping...");
      
      Stop();
      
      if (_onOpenedFunc)
        _onOpenedFunc();
    }
    else if (_doorState == Closing && digitalRead(_doorClosedSwitchPin) == DOOR_CLOSED_STATE) 
    {
      DEBUG_PRINT("Door detected closed.  Stopping...");
      
      Stop();
      
      if (_onClosedFunc)
        _onClosedFunc();
    }
    else 
    {
      if (millis() > (_doorRunStart + _maxRunTime)) 
      {
        DEBUG_PRINT("Door timed out while " + String(_doorState) + ".  Stopping...");

        // door has hit max running time
        Stop();

        if (_onMaxRunFunc)
          _onMaxRunFunc();
      }
    }
  }
  else
    _doorRunStart = 0;

  _lastDoorState = _doorState;
}

void Door::Open() 
{
  _openClose(true);
}

void Door::Close() 
{
  _openClose(false);
}

void Door::Toggle()
{
  _openClose(_doorState == Opening || _doorState == Opened ? false : true);
}

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

boolean Door::IsOpened()
{
  return _doorState == Opened;
}

boolean Door::IsClosed()
{
  return _doorState == Closed;
}

void Door::_openClose(boolean openIt) 
{
  DEBUG_PRINT("Door " + String(openIt ? "opening... " : "closing..."));

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

void Door::OnMaxRun(void (*onMaxRun)())
{
  _onMaxRunFunc = onMaxRun;
}

void Door::OnOpened(void (*onOpened)())
{
  _onOpenedFunc = onOpened;
}

void Door::OnClosed(void (*onClosed)())
{
  _onClosedFunc = onClosed;
}

void Door::OnDoorStart(void (*onDoorStart)())
{
  _onDoorStartFunc = onDoorStart;
}

