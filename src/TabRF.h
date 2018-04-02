/**
 * @file: TabRF.h
 * @brief
 * This sample is part of the TabRF library that implements receiving an sending
 * RF protocols defined in a table. The library internals are explained at
 * http://www.mathertel.de/Arduino/tabrflibrary.aspx
 */

#ifndef TabRF_H
#define TabRF_H

#include "debugout.h"
#include <Arduino.h>

#define NUL '\0'
#define null 0

// Interrupt service routine must reside in RAM
#ifndef ICACHE_RAM_ATTR
#define ICACHE_RAM_ATTR
#endif

/**
 * Code types
 * flags that specify the allowed usage of a code
 */
typedef enum {
  TabRF_CODE_ANY = 0x10, // A valid code type.
  TabRF_CODE_START = 0x01, // This code can start a sequence
  TabRF_CODE_END = 0x02 // This code can end a sequence
} TabRF_CODE_TYPE;

// a valid code may have up to MAX_CODELENTGH timings.
#define MAX_CODELENTGH 4

// The Code structure is used to hold the definition of the protocol,
// the timings and the current state information while receiving the code.
struct Code {
  TabRF_CODE_TYPE type; // type of usage of code
  char name; // single character name for this code used for the message string.
  uint8_t length; // number of timings for this code
  unsigned int time[MAX_CODELENTGH]; // average time of the code part.

  // these fields reflect the current status of the code.
  uint8_t cnt; // number of discovered timings.
  uint8_t valid; // is true while discovering and the code is still possible.
};


struct Sequence {
  // minimal number of codes in a row required by the protocol
  uint8_t minCodeLen;

  // maximum number of codes in a row defining a complete  CodeSequence.
  uint8_t maxCodeLen;

  // tolerance of the timings in percent.
  uint8_t tolerance;

  // Number of repeats when sending.
  uint8_t sendRepeat;

  // Number of defined codes in this table
  uint8_t length;

  // name, type and timings of used codes.
  Code *codes;
}; // struct Sequence


// anything code squence less than this length is not valid.
#define MIN_CODE_LEN 8

// ===== =====

typedef void (*TabRFCallbackFunction)(char *code);

// maximal number of codes in a single sequence.
#define MAXCODES 48 // at least 36 + 2

class TabRFClass;
extern TabRFClass tabRF;

// main class for the TabRF library
class TabRFClass
{
public:
  TabRFClass();

  /** attach a callback function that will get passed any new code. */
  void attachCallback(TabRFCallbackFunction newFunction);

  // send out a new code
  void send(char *code);

  // ===== debug helper functions =====

  /** Send a summary of the current code-table to the Serial output. */
  void dumpTable(Sequence *pSeq)
  {
    // dump the Sequence characteristics
    DEBUG_MSG("min. Length:%3u\n", pSeq->minCodeLen);
    DEBUG_MSG("max. Length:%3u\n", pSeq->maxCodeLen);
    DEBUG_MSG("Tolerance  :%3u\n", pSeq->tolerance);
    DEBUG_MSG("SendRepeat :%3u\n", pSeq->sendRepeat);

    uint8_t len = pSeq->length;
    Code *pTab = pSeq->codes;

    while (len > 0) {
      DEBUG_MSG("  [%c]:", pTab->name);

      for (int n = 0; n < pTab->length; n++) {
        DEBUG_MSG("%6u", pTab->time[n]);
      } // for
      DEBUG_MSG("\n");

      // next code entry
      pTab++;
      len--;
    } // while
    DEBUG_MSG("\n");
  } // dumpTable()


  // dump current active code table
  void dumpTable()
  {
    dumpTable(_sequence);
  } // dumpTable()


  /**
   * @brief Initialize receiving and sending pins and register
   * interrupt service routine.
   *
   * @param recvPin
   * @param sendPin
   */
  void init(int recvPin, int sendPin);

  void setupDefition(Sequence *codeDefinition)
  {
    _sequence = codeDefinition;
  };

private:
  /** hardware related settings */
  int _recvPin; // Pin number of receiver.
  int _sendPin; // Pin number of sender.
  int _irq; // Interrupt numbert of receiver.


  /** code table and related settings */
  Sequence *_sequence;

  TabRFCallbackFunction _callbackFunc;

  /** received codes */
  char _signals[MAXCODES];

  /** length of _signals. */
  uint8_t _signals_length; // length of current recorded protocol.


  /**
   * Reset all counters in the code-table to start next matching
   * and optional reset current received signals.
   */
  void _resetCode(bool all)
  {
    if (_sequence) {
      Code *pCode = _sequence->codes;
      int8_t n = _sequence->length;

      while (n > 0) {
        pCode->cnt = 0;
        pCode->valid = true;

        n--;
        pCode++;
      } // while

      if (all) {
        _signals_length = 0;
      } // if
    } // if
  } // _resetCode()


  // check for a timing with the given duration fits into a code.
  // and when a code is complete, check for protocol conditions start end end.
  void _state(unsigned int duration)
  {
    Code *foundCode = NULL; // a completed code sequence
    int8_t match = false;

    if (_sequence) {
      // search for all codes for a possible match at the end of the sequence
      int8_t len = _sequence->length;

      Code *thisCode = _sequence->codes;
      while ((!foundCode) && (len > 0)) {

        int8_t i = thisCode->cnt;

        if (thisCode->valid) {
          unsigned int time = thisCode->time[i];
          unsigned int tol = (time * 4) / 10;
          if ((duration < time - tol) || (duration > time + tol)) {
            // This code sequence is not matching
            thisCode->valid = false;

          } else {
            match = true;

            // this timing is matching
            thisCode->cnt = i = i + 1;

            if (i == thisCode->length) {
              // this pattern is matching and code is complete
              _signals[_signals_length++] = thisCode->name;
              foundCode = thisCode; // first match will exit the loop.

              // start matching from the beginning with next timing
              _resetCode(false);
            } // if
          } // if
        } // if
        len--;
        thisCode++;
      } // while

      if (!match) {
        // no matching code timing found => reset everything.
        _resetCode(true);

      } else if (foundCode) {
        // a complete code sequene was found

        if (_signals_length == 1) {

          if (foundCode->type != TabRF_CODE_START) {
            // first code is not a valid starting code.
            _resetCode(true); // reset all code scanning
          }

        } else if ((_signals_length > MIN_CODE_LEN) &&
                   (foundCode->type & TabRF_CODE_END)) {
          // code candidate !

          // add terminating NUL character to use _signals as a string.
          _signals[_signals_length++] = NUL;

          if (_callbackFunc)
            _callbackFunc(_signals);
          _resetCode(true);

        } else if ((_signals_length == MAXCODES - 2)) {
          // no code candidate, max Length exceeded !
          _resetCode(true); // reset current protocol
        } // if

      } // if
    } // if
  } // _state()


  // This handler is attached to the change interrupt.
  // ICACHE_RAM_ATTR
  static void signal_change_handler()
  {
    // last time the interrupt was called.
    volatile static unsigned long lastTime = micros();

    unsigned long now = micros();
    tabRF._state(now - lastTime);

    lastTime = now; // micros();
  } // signal_change_handler()
}; // class TabRFClass

#endif