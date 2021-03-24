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

#ifndef TabRF_H_
#define TabRF_H_

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

#define buf88_size 256

// main class for the TabRF library
class TabRFClass
{
public:
  /**
   * @brief Initialize receiving and sending pins and register
   * interrupt service routine.
   * @param recvPin
   * @param sendPin
   */
  void init(SignalParser *sig, int recvPin, int sendPin, int trim = 0);

  // send out a new code
  void send(const char *code, int value = 0);

  void loop();

  // ===== Insights and Debugging Helpers =====

  // Return the number of buffered data in the ring buffer.
  // This may be used to find the ring buffer is too small or loop() needs to be
  // called more often.
  uint32_t getBufferCount()
  {
    return (buf88_cnt);
  };

  /** Return the last received timings from the ring-buffer.
   * When the length is larger than the ring buffer the length is reduced.
   * There is always a 0 entry in the last timings.
   * @param buffer target timing buffer
   * @param len length of buffer
   */
  void getBufferData(SignalParser::CodeTime *buffer, int len);

  /** dump the data from a table of timings that end with a 0 time.
   * @param raw pointer to raw timings data.
  */
  void dumpTimings(SignalParser::CodeTime *raw);

private:
  // Ring buffer
  // A simple ring buffer is used to decouple interrupt routine.
  // Static variables are used to be known in the ISR
  static SignalParser::CodeTime *buf88; // allocated memory
  static volatile SignalParser::CodeTime *buf88_write; // write pointer
  static volatile SignalParser::CodeTime *buf88_read; // read pointer
  static SignalParser::CodeTime *buf88_end; // end of buffer+1 pointer for wrapping
  static volatile unsigned int buf88_cnt; // number of bytes in buffer

  SignalParser *_sig;


  /** hardware related settings */
  static int _recvPin; // IO Pin number for receiving signals. static, to be
      // known in the ISR
  int _sendPin; // IO Pin number for sendint signals.
  int _irNumber; // Interrupt number of receiver.
  static int _trim; // timming factor


  // ===== Interrupt service routine =====

  // This handler is attached to the change interrupt.
  static void ICACHE_RAM_ATTR signal_change_handler();

}; // class TabRFClass

#endif // TabRF_H_