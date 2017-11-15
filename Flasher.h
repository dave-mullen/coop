#ifndef _FLASHER_h
#define _FLASHER_h

#include "Arduino.h"

enum FlashType {
	None,
	Solid,
	Pulse,
	Blink,
	BlinkFast,
	Revert
};

class Flasher
{
 private:
	 int _ledPin;
	 FlashType _type;
	 unsigned long _flashStartTime;
	 unsigned long _flashStopTime;
	 FlashType _afterType;
	 void _writeToPin(int value); 
	 FlashType _saveType;
	 int _lastValue;
 public:
	Flasher();
	void Init(int ledPin);
	boolean Update();
	void SetFlash(enum FlashType type);
	void SetFlashSequence(enum FlashType type, int time, enum FlashType afterType);
	boolean IsType(enum FlashType type);
    // Events
    void OnPulseComplete(void (*onPulseComplete)());
};

#endif

