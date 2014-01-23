//#define DEBUG
#include "DebugUtils.h"

#include "Arduino.h"
#include "Button.h"

const int BUTTON_DEBOUNCE_TIME = 100;
const int BUTTON_HOLD_MILLIS = 3000;

void (*_onPressFunc)();
void (*_onHoldFunc)();

Button::Button() 
{
}

void Button::Init(int buttonPin)
{
  _buttonPin = buttonPin;

  pinMode(_buttonPin, INPUT);
}

void Button::Update()
{
  long currentMillis = millis();
  
  _pressedState = digitalRead(_buttonPin) == HIGH;
  
  if (!_pressedState) 
  {
    if (_lastPressedState && currentMillis > (_pressedStart + BUTTON_DEBOUNCE_TIME))
    {
      DEBUG_PRINT("Let go.  Current time is " + String(millis()) + ".  Greater than " + String(_pressedStart + BUTTON_HOLD_MILLIS) + "?");
      // User has just let go of button.  Decide if it was a press or hold and throw appropriate event
      if (_onHoldFunc && (currentMillis > (_pressedStart + BUTTON_HOLD_MILLIS)))
        _onHoldFunc();
      else if (_onPressFunc)
        _onPressFunc();
    }
    else
      _pressedStart = 0;
  }
  else if (!_lastPressedState)
  {
    // User has pressed button.  Start timer...
    _pressedStart = currentMillis;
    DEBUG_PRINT("Set _pressedStart to " + String(_pressedStart));
  }

  _lastPressedState = _pressedState;
}


void Button::OnPress(void (*onPress)())
{
  _onPressFunc = onPress;
}

void Button::OnHold(void (*onHold)())
{
  _onHoldFunc = onHold;
}


