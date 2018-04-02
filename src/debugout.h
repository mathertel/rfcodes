// Debug output helper 

#ifndef DEBUGOUT_H
#define DEBUGOUT_H

#ifdef DEBUG_ESP_PORT
#define DEBUG_MSG(...) DEBUG_ESP_PORT.printf(__VA_ARGS__)
#else
#define DEBUG_MSG(...)
#endif

#endif
