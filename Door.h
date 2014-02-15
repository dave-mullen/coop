#ifndef _DOOR_H
#define _DOOR_H

#include "Arduino.h"

class Door
{
  public:
	Door();
    void Init(int doorOpenedSwitchPin, int doorClosedSwitchPin, int motorStbyPin, int motorPwmPin, int motorIn1Pin, int motorIn2Pin, byte doorSpeed, int maxOpenTime, int maxCloseTime);
    boolean Update();
    void Open();
    void Close();
    void Toggle();
    void Stop();
    boolean IsOpened();
    boolean IsClosed();
    
    // Events
    void OnMaxRun(void (*onMaxRun)());
    void OnOpened(void (*onOpened)());
    void OnClosed(void (*onClosed)());
    void OnDoorStart(void (*onDoorStart)());

  private:
    enum DoorStateType {
      Idle,
      Opening,
      Closing,
      Opened,
      Closed
    };

    DoorStateType _doorState;
    DoorStateType _lastDoorState;
    long _doorRunStart;
    int _doorOpenedSwitchPin;
    int _doorClosedSwitchPin;
    int _motorStbyPin;
    int _motorPwmPin;
    int _motorIn1Pin;
    int _motorIn2Pin;
    byte _doorSpeed;
    int _maxOpenTime;
	int _maxCloseTime;
    
    void _openClose(boolean openIt);
};

#endif
