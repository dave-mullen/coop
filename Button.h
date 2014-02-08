#ifndef _BUTTON_H
#define _BUTTON_H

#include "Arduino.h"

class Button
{
  public:
    Button();
    void Init(int buttonPin);
    boolean Update();
    
    // Events
    void OnPress(void (*onPress)());
    void OnHold(void (*onHold)());
  private:
    boolean _pressedState;
    boolean _lastPressedState;
    long _pressedStart;
    int _buttonPin;
    int _holdTime;
};

#endif
