/**
 * @Authors         Sebastiaan den Hertog
 * @Date created    04-02-2025
 * @Date updated    04-02-2025 (By: Sebastiaan den Hertog)
 * @Description     Constants file for the project
 */

#ifndef CONST_H
#define CONST_H
#include <Arduino.h>

#define ESP_DEBUG
// #define ESP_DEBUG_FS

#ifndef MAX_LED_MEMORY
#define MAX_LED_MEMORY 64000
#endif

#define MAX_PINS 10

#define DEBUGOUT Serial

#ifdef ESP_DEBUG
#define DEBUG_PRINT(x) DEBUGOUT.print(x)
#define DEBUG_PRINTLN(x) DEBUGOUT.println(x)
#define DEBUG_PRINTF(x...) DEBUGOUT.printf(x)
#else
#define DEBUG_PRINT(x)
#define DEBUG_PRINTLN(x)
#define DEBUG_PRINTF(x...)
#endif

#ifdef ESP_DEBUG_FS
#define DEBUGFS_PRINT(x) DEBUGOUT.print(x)
#define DEBUGFS_PRINTLN(x) DEBUGOUT.println(x)
#define DEBUGFS_PRINTF(x...) DEBUGOUT.printf(x)
#else
#define DEBUGFS_PRINT(x)
#define DEBUGFS_PRINTLN(x)
#define DEBUGFS_PRINTF(x...)
#endif

#endif // CONST_H
