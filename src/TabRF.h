/**
 * @file: TabRF.h
 * @brief
 * This sample is part of the TabRF library that implements receiving an sending
 * RF protocols defined in a table.
 *
 * This work is licensed under a BSD style license,
 * https://www.mathertel.de/License.aspx.
 *
 * More information on http://www.mathertel.de/Arduino/tabrflibrary.aspx
 *
 * Changelog:
 * * 29.04.2018 created by Matthias Hertel
 * * 06.08.2018 const char send, allow for sending only.
 */

#ifndef TabRF_H
#define TabRF_H

// define RAWTIMES with the maximum of captured raw timing codes
#define RAWTIMES 500

#include <Arduino.h>

#include "debugout.h"
#include "signal_parser.h"

#define NUL '\0'
#define null 0

// Interrupt service routine must reside in RAM in ESP8266
#ifndef ICACHE_RAM_ATTR
#define ICACHE_RAM_ATTR
#endif

#define NO_PIN (-1)

#define TabRF_ERR(...) Serial.printf("Error: " __VA_ARGS__)

#if defined(RAWTIMES)
extern uint32_t probes[RAWTIMES];
extern uint32_t probesCount;
#endif

// main class for the TabRF library
class TabRFClass {
 public:
  /**
   * @brief Initialize receiving and sending pins and register
   * interrupt service routine.
   * @param recvPin
   * @param sendPin
   */
  void init(SignalParser *sig, int recvPin, int sendPin);

  // send out a new code
  void send(const char *code, int value = 0);

  void loop();

  uint32_t getBufferCount(){return (buf88_cnt);};

#ifdef RAWTIMES
  void dumpRawTimes();
#endif

 private:
#define buf88_size 256
  // simple ring buffer to decouple interrupt routine
  static uint32_t *buf88;                 // allocated memory
  static volatile uint32_t *buf88_write;  // write pointer
  static volatile uint32_t *buf88_read;   // read pointer
  static uint32_t *buf88_end;          // end of buffer+1 pointer for wrapping
  static volatile uint32_t buf88_cnt;  // number of bytes in buffer

  SignalParser *_sig;

  /** hardware related settings */
  int _recvPin;   // IO Pin number for receiving signals.
  int _sendPin;   // IO Pin number for sendint signals.
  int _irNumber;  // Interrupt number of receiver.

  // ===== debug helper functions =====

#ifdef RAWTIMES
  /** received codes */
  uint32_t raw[RAWTIMES];
  uint32_t rawCount;
#endif

  // This handler is attached to the change interrupt.
  static void ICACHE_RAM_ATTR signal_change_handler() {
    // last time the interrupt was called.
    volatile static unsigned long lastTime = micros();

    unsigned long now = micros();

    // write to ring buffer
    if (TabRFClass::buf88_cnt < buf88_size) {
      *TabRFClass::buf88_write++ = (uint32_t)(now - lastTime);
      buf88_cnt++;

      // reset pointer to the start when reaching end
      if (TabRFClass::buf88_write == TabRFClass::buf88_end)
        TabRFClass::buf88_write = TabRFClass::buf88;
    }  // if

    lastTime = now;  // micros();
  }  // signal_change_handler()

};  // class TabRFClass

#endif