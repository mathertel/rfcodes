// Debug output helper

#ifndef DEBUGOUT_H_
#define DEBUGOUT_H_

// #define NODEBUG

#if defined(NODEBUG)
#define TRACE_MSG(...)
#define ERROR_MSG(...)
#define INFO_MSG(...)
#define RAW_MSG(...)

#elif defined(DEBUG_ESP_PORT)
// ESP8266 way to specify the text output e.g. using Serial
#define ERROR_MSG(...) { DEBUG_ESP_PORT.printf(__VA_ARGS__); DEBUG_ESP_PORT.println(); }
#define INFO_MSG(...)  { DEBUG_ESP_PORT.printf(__VA_ARGS__); DEBUG_ESP_PORT.println(); }
#define TRACE_MSG(...) { DEBUG_ESP_PORT.printf(__VA_ARGS__); DEBUG_ESP_PORT.println(); }
#define RAW_MSG(...)   { DEBUG_ESP_PORT.printf(__VA_ARGS__); }

#else
#include <cstdio>
// stdout version
#define ERROR_MSG(fmt, ...) fprintf(stderr, "[error] " fmt "\n", __VA_ARGS__);
#define INFO_MSG(fmt, ...) fprintf(stderr, "[info]  " fmt "\n", __VA_ARGS__);
#define TRACE_MSG(fmt, ...) fprintf(stderr, "[trace] " fmt "\n", __VA_ARGS__);
#define RAW_MSG(...)   fprintf(stderr, __VA_ARGS__);
#endif

#undef TRACE_MSG
#define TRACE_MSG(...)


#endif // DEBUGOUT_H_
