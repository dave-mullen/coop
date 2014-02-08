#ifndef _DEBUGUTILS_H
#define _DEBUGUTILS_H

#include "Arduino.h"

#ifdef DEBUG
#define DEBUG_PRINT(str)    \
    Serial.print(str);
#define DEBUG_PRINTLN(str)    \
    Serial.println(str);
#else
#define DEBUG_PRINT(str)
#define DEBUG_PRINTLN(str)
#endif

#endif
