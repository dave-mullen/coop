#ifndef _DEBUGUTILS_H
#define _DEBUGUTILS_H

#include "Arduino.h"

#ifdef DEBUG
#define DEBUG_PRINT(str)    \
    Serial.println(str);
#else
#define DEBUG_PRINT(str)
#endif

#endif
