#include "LowPower.h"
#include <avr/sleep.h>
#include <avr/power.h>
#include <avr/wdt.h>

#include "Flasher.h"
//#define DEBUG
#include "DebugUtils.h"

#include "Arduino.h"
#include <EEPROM.h>
#include "Door.h"
#include "Button.h"
#include "DaylightSensor.h"
#include "Flasher.h"
#include "LowPower.h"

// Pin number constants
const int PIN_DOOR_OPENED_SWITCH = 8; // Door open micro-switch
const int PIN_DOOR_CLOSED_SWITCH = 9; // Door closed micro-switch
const int PIN_DOOR_TOGGLE_BUTTON = 13; // Open/close door button
const int PIN_LIGHT_SENSOR = A0; // Photo resistor (analog)
const int PIN_MOTOR_STBY = 6; // Motor standby
const int PIN_MOTOR_A_PWM = 3; // Motor speed (PWM)
const int PIN_MOTOR_A_IN1 = 4; // Direction 1
const int PIN_MOTOR_A_IN2 = 5; // Direction 2
const int PIN_FLASHER = 11; // Alert LED (PWM)

// Configuration constants
const long CONFIG_DAYLIGHT_CHANGE_DELAY = 300000; // 5 min Delay (ms) to recognize that it is actually a day/night change and not just temporary
const byte CONFIG_DOOR_SPEED = 255; // Speed at which the door motor should run
const int CONFIG_DOOR_MAX_OPEN_TIME = 32000; // Milliseconds the door should be allowed to take to open
const int CONFIG_DOOR_MAX_CLOSE_TIME = 24000; // Milliseconds the door should be allowed to take to close
const long SLEEP_MODE_TIME = 120000; // Milliseconds of inactivity before starting sleep mode

Door door;
Button toggleButton;
DaylightSensor daylightSensor;
Flasher flasher;

// the setup routine runs once when you press reset:
void setup() {
  // TODO - initialize pins
	Serial.begin(9600);

	door.Init(PIN_DOOR_OPENED_SWITCH, PIN_DOOR_CLOSED_SWITCH, PIN_MOTOR_STBY, PIN_MOTOR_A_PWM, PIN_MOTOR_A_IN1, PIN_MOTOR_A_IN2, CONFIG_DOOR_SPEED, CONFIG_DOOR_MAX_OPEN_TIME, CONFIG_DOOR_MAX_CLOSE_TIME);
 
	door.OnMaxRun(DoorMaxRun);
	door.OnOpened(DoorOpened);
	door.OnClosed(DoorClosed);
	door.OnDoorStart(DoorStart);

	toggleButton.Init(PIN_DOOR_TOGGLE_BUTTON);
	toggleButton.OnPress(ToggleButtonPress);
	toggleButton.OnHold(ToggleButtonHold);
  
	daylightSensor.Init(PIN_LIGHT_SENSOR, door.IsOpened(), CONFIG_DAYLIGHT_CHANGE_DELAY);
	daylightSensor.OnChanging(DaylightSensorChanging);
	daylightSensor.OnCancelled(DaylightSensorCancelled);
	daylightSensor.OnChange(DaylightSensorChange);
	daylightSensor.OnSetLevel(DaylightSensorSetLevel);
  
	flasher.Init(PIN_FLASHER);
	flasher.OnPulseComplete(FlasherPulseComplete);
}

// the loop routine runs over and over again forever:
void loop() 
{
	static long lastUpdateMillis = millis();
	long currentMillis = millis();

	// Run all event processors.  They will return true if something is happening
	boolean hadUpdate = false;
	hadUpdate |= door.Update();
	hadUpdate |= toggleButton.Update();
	hadUpdate |= daylightSensor.Update();
	hadUpdate |= flasher.Update();

	// Stamp time of last activity or handle overflow
	if (hadUpdate || currentMillis < lastUpdateMillis)
		lastUpdateMillis = currentMillis;

	// If no activity see if it has been idle for SLEEP_MODE_TIME ms
	if (!hadUpdate && (currentMillis - lastUpdateMillis) > SLEEP_MODE_TIME)
	{
		if (daylightSensor.IsDaytime())
		{
			// If daytime we will sleep for a very short time to save battery but still be ready to process events
			LowPower.powerDown(SLEEP_120MS, ADC_OFF, BOD_OFF); 
			DEBUG_PRINT("z");
			delay(3); 
		}
		else
		{
			// At nighttime set pulse flasher which will trigger longer sleep after each pulse
			flasher.SetFlash(Pulse);
		}
	}
}


// Door event handlers

void DoorMaxRun() 
{
	DEBUG_PRINTLN("DoorMaxRun event.  IsOpen = " + String(door.IsOpened()) + ", IsClosed = " + String(door.IsClosed()));
	daylightSensor.Reset(!door.IsClosed());
	flasher.SetFlash(BlinkFast);
}

void DoorOpened() 
{
	DEBUG_PRINTLN("DoorOpened event");
	daylightSensor.Reset(true);
	flasher.SetFlash(None);
}

void DoorClosed() 
{
	DEBUG_PRINTLN("DoorClosed event");
	daylightSensor.Reset(false);
	flasher.SetFlash(None);
}

void DoorStart()
{
	// TODO - flash LED
	DEBUG_PRINTLN("DoorStart event");
	flasher.SetFlash(Solid);
}

// Toggle button event handlers
void ToggleButtonPress()
{
	boolean isOpened = door.IsOpened() || !door.IsClosed();
	door.Toggle();
	DEBUG_PRINTLN("ToggleButtonPress event");
}

void ToggleButtonHold()
{
	DEBUG_PRINTLN("ToggleButtonHold event");

	// Save the current daylight level to EEPROM
	if (door.IsOpened())
		daylightSensor.SetDayLevel();
	else if (door.IsClosed())
		daylightSensor.SetNightLevel();
}

// Daylight sensor events
void DaylightSensorChanging()
{
	DEBUG_PRINTLN("DaylightSensorChanging event");

	flasher.SetFlash(Blink);
}

void DaylightSensorCancelled()
{
	DEBUG_PRINTLN("DaylightSensorCancelled event");

    flasher.SetFlash(Revert);
}

void DaylightSensorChange(boolean isDaylight)
{
	DEBUG_PRINTLN("DaylightSensorChange event");

	if (isDaylight)  
		door.Open();
	else
		door.Close();
}

void DaylightSensorSetLevel() 
{
	DEBUG_PRINTLN("DaylightSensorSetLevel event");
	flasher.SetFlashSequence(BlinkFast, 2000, Revert);
}

/// FlasherPulseComplete - Thrown when the flasher completes a pulse.  Tells us it's ok to sleep for 8 seconds
void FlasherPulseComplete()
{
#ifdef DEBUG
	delay(100); // Clear serial buffer
#endif

	LowPower.powerDown(SLEEP_8S, ADC_OFF, BOD_OFF); 
	DEBUG_PRINT("ZZZ");
}
