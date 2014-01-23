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

//// Macros for sleep mode
//#ifndef cbi
//#define cbi(sfr, bit) (_SFR_BYTE(sfr) &= ~_BV(bit))
//#endif
//#ifndef sbi
//#define sbi(sfr, bit) (_SFR_BYTE(sfr) |= _BV(bit))
//#endif


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
const int CONFIG_NIGHT_LEVEL = 15; // Light sensor level at which we consider it nighttime
const long CONFIG_DAYLIGHT_CHANGE_DELAY = 300000; // 5 min Delay (ms) to recognize that it is actually nighttime
//const int CONFIG_DAY_LEVEL = 50; // Light sensor level at which we consider it daytime
//const long CONFIG_DAYLIGHT_CHANGE_DELAY = 10000; //15 * 60 * 1000; // Delay (ms) to recognize that it is actually daytime
const byte CONFIG_DOOR_SPEED = 255; // Speed at which the door motor should run
const int CONFIG_DOOR_MAX_RUN = 30000; // Milliseconds the door should be allowed to run for
const long SLEEP_MODE_TIME = 20000; // Milliseconds of inactivity before starting sleep mode
Door door;
Button toggleButton;
DaylightSensor daylightSensor;
Flasher flasher;

long lastEventMillis = 1;
boolean sleepModeSafe = false;

// the setup routine runs once when you press reset:
void setup() {
  // TODO - initialize pins
	Serial.begin(9600);

	door.Init(PIN_DOOR_OPENED_SWITCH, PIN_DOOR_CLOSED_SWITCH, PIN_MOTOR_STBY, PIN_MOTOR_A_PWM, PIN_MOTOR_A_IN1, PIN_MOTOR_A_IN2, CONFIG_DOOR_SPEED, CONFIG_DOOR_MAX_RUN);
 
	door.OnMaxRun(DoorMaxRun);
	door.OnOpened(DoorOpened);
	door.OnClosed(DoorClosed);
	door.OnDoorStart(DoorStart);

	//isDaytime = door.IsOpened();
  
	toggleButton.Init(PIN_DOOR_TOGGLE_BUTTON);
	toggleButton.OnPress(ToggleButtonPress);
	toggleButton.OnHold(ToggleButtonHold);
  
	daylightSensor.Init(PIN_LIGHT_SENSOR, door.IsOpened(), CONFIG_DAYLIGHT_CHANGE_DELAY);
	daylightSensor.OnChanging(DaylightSensorChanging);
	daylightSensor.OnCancelled(DaylightSensorCancelled);
	daylightSensor.OnChange(DaylightSensorChange);
	daylightSensor.OnSetLevel(DaylightSensorSetLevel);
  
	flasher.Init(PIN_FLASHER);

	//cbi(SMCR,SE );      // sleep enable, power down mode
	//cbi(SMCR,SM0 );     // power down mode
	//sbi(SMCR,SM1 );     // power down mode
	//cbi(SMCR,SM2 );     // power down mode

	//SetupSleepMode(5);

}

// the loop routine runs over and over again forever:
void loop() {
	door.Update();
	toggleButton.Update();
	daylightSensor.Update();
	flasher.Update();

	long currentMillis = millis();
	if (currentMillis < lastEventMillis)
		lastEventMillis = currentMillis;

	// No event has run for 20 seconds
	if ((currentMillis - lastEventMillis) > SLEEP_MODE_TIME && flasher.IsIdle())
	{
		DEBUG_PRINT("Going to sleep " + String(currentMillis) + ", " + String(lastEventMillis));
		//delay(80);
		//GoToSleep();
		//LowPower.powerDown(SLEEP_120MS, ADC_OFF, BOD_OFF); 
		LowPower.idle(SLEEP_500MS, ADC_OFF, TIMER2_OFF, TIMER1_OFF, TIMER0_ON, SPI_OFF, USART0_OFF, TWI_OFF);
	}
}


// Door event handlers

void DoorMaxRun() 
{
	EventTick();

	// TODO - LED warning
	DEBUG_PRINT("DoorMaxRun event");
	DEBUG_PRINT("IsOpen = " + String(door.IsOpened()) + ", IsClosed = " + String(door.IsClosed()));
	daylightSensor.Reset(!door.IsClosed());
	flasher.SetFlash(BlinkFast);
}

void DoorOpened() 
{
	EventTick();

	// TODO - turn LED off
	DEBUG_PRINT("DoorOpened event");
	daylightSensor.Reset(true);
	flasher.SetFlash(None);
}

void DoorClosed() 
{
	EventTick();

	// TODO - if nighttime should set appropriate LED
	DEBUG_PRINT("DoorClosed event");
	daylightSensor.Reset(false);
	flasher.SetFlash(Pulse);
}

void DoorStart()
{
	EventTick();

	// TODO - flash LED
	DEBUG_PRINT("DoorStart event");
	flasher.SetFlash(Solid);
}

// Toggle button event handlers
void ToggleButtonPress()
{
	EventTick();

	boolean isOpened = door.IsOpened() || !door.IsClosed();
	door.Toggle();
	DEBUG_PRINT("ToggleButtonPress event");
}

void ToggleButtonHold()
{
	EventTick();

	DEBUG_PRINT("ToggleButtonHold event");

	// Save the current daylight level to EEPROM
	if (door.IsOpened())
		daylightSensor.SetDayLevel();
	else if (door.IsClosed())
		daylightSensor.SetNightLevel();
}

// Daylight sensor events
void DaylightSensorChanging()
{
	EventTick();

	DEBUG_PRINT("DaylightSensorChanging event");

	flasher.SetFlash(Blink);
}

void DaylightSensorCancelled()
{
	DEBUG_PRINT("DaylightSensorCancelled event");

    flasher.SetFlash(Revert);
}

void DaylightSensorChange(boolean isDaylight)
{
	EventTick();

	DEBUG_PRINT("DaylightSensorChange event");

	if (isDaylight)  
		door.Open();
	else
		door.Close();
    
    flasher.SetFlash(daylightSensor.IsDaytime() ? None : Pulse);
}

void DaylightSensorSetLevel() 
{
	EventTick();

	DEBUG_PRINT("DaylightSensorSetLevel event");
	flasher.SetFlashSequence(BlinkFast, 2000, Revert);
}

void EventTick()
{
	lastEventMillis = millis();
}

//void GoToSleep() {
//	cbi(ADCSRA, ADEN);                    // switch Analog to Digitalconverter OFF
//
//	set_sleep_mode(SLEEP_MODE_PWR_DOWN); // sleep mode is set here
//	sleep_enable();
//
//	sleep_mode();                        // System sleeps here
//
//    sleep_disable();                     // System continues execution here when watchdog timed out 
//    sbi(ADCSRA, ADEN);                    // switch Analog to Digitalconverter ON
//}
//
//// 0=16ms, 1=32ms,2=64ms,3=128ms,4=250ms,5=500ms,6=1 sec,7=2 sec, 8=4 sec, 9= 8sec
//void SetupSleepMode(int ii) {
//  byte bb;
//  int ww;
//  if (ii > 9 ) ii=9;
//  bb=ii & 7;
//  if (ii > 7) bb|= (1<<5);
//  bb|= (1<<WDCE);
//  ww=bb;
//  Serial.println(ww);
//
//  MCUSR &= ~(1<<WDRF);
//  // start timed sequence
//  WDTCSR |= (1<<WDCE) | (1<<WDE);
//  // set new watchdog timeout value
//  WDTCSR = bb;
//  WDTCSR |= _BV(WDIE);
//}
//
////****************************************************************  
//// Watchdog Interrupt Service / is executed when  watchdog timed out
//ISR(WDT_vect) {
//	/* nothing */
//}
//
//
