#ifndef _DAYLIGHTSENSOR_H
#define _DAYLIGHTSENSOR_H

#include "Arduino.h"

class DaylightSensor
{
  public:
    DaylightSensor();
    void Init(int prPin, boolean isDaytime, long delayTime);
    void Update();
    void Reset(boolean isDaytime);
    boolean IsDaytime();
    void SetDayLevel();
    void SetNightLevel();
    
    // Events
    void OnChanging(void (*onChanging)());
    void OnCancelled(void (*onCancelled)());
    void OnChange(void (*onChange)(boolean isDaytime));
	void OnSetLevel(void (*onSetLevel)());
  private:
    byte _currentLevel();
    boolean _checkSampleTime();
    
    byte _lastLevel;
    int _prPin;
    boolean _isDaytime;
    long _changeStart;
    byte _dayLevel;
    byte _nightLevel;
    long _delayTime;
};

#endif
