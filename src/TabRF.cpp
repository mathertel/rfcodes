

#include "TabRF.h"

#include "Arduino.h"
#include "debugout.h"

// ====== TabRFClass implemenation =====

/**
 * Initialize the receiving and sending modes and activate the IO pins
 * @param recvPin The IO pin to be used for receiving. Set to -1 to disable
 * receiving mode.
 * @param sendPin The IO pin to be used for sending. Set to -1 to disable
 * sending mode.
 */
void TabRFClass::init(SignalParser *sig, int recvPin, int sendPin) {
  TRACE_MSG("Initalizing tabRF hardware\n");

  _sig = sig;

  // Receiving mode
  _recvPin = recvPin;
  if (recvPin >= 0) {
    // initialize interrupt service routine
    // See
    // https://www.arduino.cc/reference/en/language/functions/external-interrupts/attachinterrupt/
    _irNumber =
        digitalPinToInterrupt(recvPin);  // will return -1 on wrong pin number.
    if (_irNumber < 0) {
      TRACE_MSG("Error: Receiving pin cannot be used");
      _recvPin = -1;

    } else {
      pinMode(_recvPin, INPUT);
      // sig.???  _resetCode(true);
      attachInterrupt(_irNumber, signal_change_handler, CHANGE);
    }  // if
  }

  // Sending mode
  _sendPin = sendPin;
  if (sendPin >= 0) {
    // initialize sending mode
    pinMode(_sendPin, OUTPUT);
    digitalWrite(_sendPin, LOW);
  }
}  // init()

void TabRFClass::send(const char *signal, int value) {
  uint32_t timings[MAX_SEQUENCE_LENGTH];
  int level = LOW;  // LOW level before starting.

  if (_sendPin >= 0) {
    if (_recvPin >= 0) detachInterrupt(_irNumber);

    // get timings of the code
    _sig->compose(signal, timings, sizeof(timings));
    uint32_t *t = timings;

    while (*t) {
      level = !level;
      digitalWrite(_sendPin, level);
      delayMicroseconds(*t++);

    }  // while

    if (_recvPin >= 0)
      attachInterrupt(_irNumber, signal_change_handler, CHANGE);
  }  // if
}  // send()

void TabRFClass::loop() {
  // process bytes from ring buffer
  if (TabRFClass::buf88_cnt > 0) {
    uint32_t t = *TabRFClass::buf88_read++;
    TabRFClass::buf88_cnt--;

    _sig->parse(t);

    // add to debug buffer

    // reset pointer to the start when reaching end
    if (TabRFClass::buf88_read == TabRFClass::buf88_end)
      TabRFClass::buf88_read = TabRFClass::buf88;
  }  // if
}  // loop

#ifdef RAWTIMES
  void TabRFClass::dumpRawTimes() {
    for (int i = 0; i < rawCount; i++) {
      Serial.print(raw[i]);
      if (i % 50 == 49)
        Serial.println(',');
      else
        Serial.print(',');
    }
    Serial.println();
  }
#endif


// static class stuff, to be accessible to the Interrupt service routines.

uint32_t *TabRFClass::buf88 =
    (uint32_t *)malloc(buf88_size * sizeof(uint32_t));  // allocated memory
volatile uint32_t *TabRFClass::buf88_write =
    TabRFClass::buf88;                                          // write pointer
volatile uint32_t *TabRFClass::buf88_read = TabRFClass::buf88;  // read pointer
uint32_t *TabRFClass::buf88_end =
    TabRFClass::buf88 + buf88_size;  // end of buffer+1 pointer for wrapping
volatile uint32_t TabRFClass::buf88_cnt = 0;  // number of bytes in buffer

// End.
