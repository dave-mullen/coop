//#define DEBUG
#include "DebugUtils.h"

#include "Arduino.h"
#include "Button.h"

#pragma region Constants

const long BUTTON_DEBOUNCE_TIME = 100;
const long BUTTON_HOLD_MILLIS = 3000;

#pragma endregion

#pragma region Private Event Handlers

void (*_onPressFunc)();
void (*_onHoldFunc)();

#pragma endregion

#pragma region Constructor

Button::Button() 
{
}

#pragma endregion

#pragma region Public Methods

/// <summary>Button::Init - Initialize Button object.</summary>
void Button::Init(int buttonPin)
{
  _buttonPin = buttonPin;

  pinMode(_buttonPin, INPUT);
}

/// <summary>Button::Update - Event processor for Button.</summary>
boolean Button::Update()
{
	boolean hasUpdate = true;
	long currentMillis = millis();
  
	_pressedState = digitalRead(_buttonPin) == HIGH;
  
	if (!_pressedState) 
	{
		// Button not pressed.  See if it was pressed last time around and for how long
		if (_lastPressedState && currentMillis > (_pressedStart + BUTTON_DEBOUNCE_TIME))
		{
			DEBUG_PRINTLN("Let go of button.  Hold time was " + String(currentMillis - _pressedStart) + "ms");
			// User has just let go of button.  Decide if it was a press or hold and throw appropriate event
			if (_onHoldFunc && (currentMillis > (_pressedStart + BUTTON_HOLD_MILLIS)))
				_onHoldFunc();
			else if (_onPressFunc)
				_onPressFunc();
		}
		else if (_lastPressedState)
		{
			// Button press was too quick
			DEBUG_PRINTLN("Button press too short.  Ignoring...");
		}
		else 
		{
			// Wasn't pressed before - nothing happened
			_pressedStart = 0;
			hasUpdate = false;
		}
	}
	else if (!_lastPressedState)
	{
		// Button pressed and wasn't pressed last update.  Save off start time for timing...
		_pressedStart = currentMillis;
		DEBUG_PRINTLN("Set _pressedStart to " + String(_pressedStart));
	}
	else // Button pressed and was also pressed last update.  Report an update so we stay awake.
		hasUpdate = true;

  _lastPressedState = _pressedState;

  return hasUpdate;
}

#pragma endregion

#pragma region Events

/// <summary>Button::OnPress - Set event handler for button press event.</summary>
void Button::OnPress(void (*onPress)())
{
  _onPressFunc = onPress;
}

/// <summary>Button::OnHold - Set event handler for button hold event.</summary>
void Button::OnHold(void (*onHold)())
{
  _onHoldFunc = onHold;
}

#pragma endregion

