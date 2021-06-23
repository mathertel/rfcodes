/**
 * @file SignalCollector.cpp
 * @brief
 * This file is part of the RFCodes library that implements receiving an sending
 * RF and IR protocols.
 *
 * This work is licensed under a BSD 3-Clause style license,
 * https://www.mathertel.de/License.aspx.
 *
 * More information on http://www.mathertel.de/Arduino
 *
*/

#include "SignalCollector.h"

// ====== SignalCollector implemenation =====

/**
 * Initialize the receiving and sending modes and activate the IO pins
 * @param recvPin The IO pin to be used for receiving. Set to -1 to disable
 * receiving mode.
 * @param sendPin The IO pin to be used for sending. Set to -1 to disable
 * sending mode.
 */
void SignalCollector::init(SignalParser *sig, int recvPin, int sendPin, int trim)
{
  TRACE_MSG("Initalizing tabRF hardware\n");

  _sig = sig;

  // Receiving mode
  _recvPin = recvPin;
  if (recvPin >= 0) {
    // initialize interrupt service routine
    // See https://www.arduino.cc/reference/en/language/functions/external-interrupts/attachinterrupt/
    _irNumber = digitalPinToInterrupt(recvPin); // will return -1 on wrong pin number.
    if (_irNumber < 0) {
      TRACE_MSG("Error: Receiving pin cannot be used");
      _recvPin = -1;

    } else {
      pinMode(_recvPin, INPUT);
      attachInterrupt(_irNumber, signal_change_handler, CHANGE);
    } // if
  }

  // Sending mode
  _sendPin = sendPin;
  if (sendPin >= 0) {
    // initialize sending mode
    pinMode(_sendPin, OUTPUT);
    digitalWrite(_sendPin, LOW);
  }
} // init()


void strcpyProtname(char *target, const char *signal)
{
  char *p = target;
  char *s = (char *)signal;
  int len = PROTNAME_LEN - 1;
  while (len && *s && (*s != ' ')) {
    *p++ = *s++;
    len--;
  }
  *p = NUL;
} // strcpyProtname


void SignalCollector::send(const char *signal)
{
  SignalParser::CodeTime timings[256];
  int level = LOW; // LOW level before starting.
  // INFO_MSG("send(%s)", signal);

  char protname[PROTNAME_LEN];
  strcpyProtname(protname, (char *)signal);
  // INFO_MSG("send prot %s", protname);

  int repeat = _sig->getSendRepeat(protname);
  // INFO_MSG("send repeat %d", repeat);

  if ((repeat) && (_sendPin >= 0)) {
    // get timings of the code
    _sig->compose(signal, timings, sizeof(timings) / sizeof(SignalParser::CodeTime));
    // dumpTimings(timings);

    while (repeat) {
      SignalParser::CodeTime *t = timings;

      if (_recvPin >= 0) {
        noInterrupts();
      }

      while (*t) {
        level = !level;
        digitalWrite(_sendPin, level);
        delayMicroseconds(*t++);
      } // while

      if (_recvPin >= 0) {
        interrupts();
      }

      repeat--;
    } // while
    
    // never leave active after sending.
    digitalWrite(_sendPin, LOW);

  } // if
} // send()


// process bytes from ring buffer
void SignalCollector::loop()
{
  while (SignalCollector::buf88_cnt > 0) {
    SignalParser::CodeTime t = *SignalCollector::buf88_read++;
    SignalCollector::buf88_cnt--;

    _sig->parse(t);

    // reset pointer to the start when reaching end
    if (SignalCollector::buf88_read == SignalCollector::buf88_end)
      SignalCollector::buf88_read = SignalCollector::buf88;
    yield();
  } // while
} // loop


// ===== Insights and Debugging Helpers =====


/** Return the last received timings from the ring-buffer. */
void SignalCollector::getBufferData(SignalParser::CodeTime *buffer, int len)
{
  len--; // keep space for final '0';
  if (len > SC_BUFFERSIZE)
    len = SC_BUFFERSIZE;

  SignalParser::CodeTime *p = (SignalParser::CodeTime *)buf88_read - len;
  if (p < buf88)
    p += SC_BUFFERSIZE;

  // copy timings to buffer
  while (len) {
    *buffer++ = *p++;

    // reset pointer to the start when reaching end
    if (p == SignalCollector::buf88_end)
      p = SignalCollector::buf88;
    len--;
  } // if
  *buffer = 0;
}; // getBufferData()


/** dump the data from a table of timings that end with a 0 time. */
void SignalCollector::dumpTimings(SignalParser::CodeTime *raw)
{
  // dump probes
  SignalParser::CodeTime *p = raw;
  int len = 0;
  while (p && *p) {
    if (len % 8 == 0) {
      RAW_MSG("%3d: %5u,", len, *p);
    } else if (len % 8 == 7) {
      RAW_MSG(" %5u,\n", *p);
    } else {
      RAW_MSG(" %5u,", *p);
    }
    p++;
    len++;
  } // while
  RAW_MSG("\n");
} // dumpTimings


// static class stuff, to be accessible to the Interrupt service routines.

// This handler is attached to the change interrupt.
void IRAM_ATTR SignalCollector::signal_change_handler()
{
  unsigned long now = micros();
  SignalParser::CodeTime t = (SignalParser::CodeTime)(now - SignalCollector::lastTime);

  // // adjust the timing with the trim factor.
  // int level = digitalRead(_recvPin);
  // if (level) {
  //   t += SignalCollector::_trim; // end of low
  // } else {
  //   t -= SignalCollector::_trim; // end of high
  // }

  // write to ring buffer
  if (SignalCollector::buf88_cnt < SC_BUFFERSIZE) {
    *SignalCollector::ringWrite++ = t;
    buf88_cnt++;

    // reset pointer to the start when reaching end
    if (SignalCollector::ringWrite == SignalCollector::buf88_end)
      SignalCollector::ringWrite = SignalCollector::buf88;
  } // if

  lastTime = now; // micros();
} // signal_change_handler()


// Inject a test timing into the ring buffer.
void SignalCollector::injectTiming(SignalParser::CodeTime t)
{
  // write to ring buffer
  if (SignalCollector::buf88_cnt < SC_BUFFERSIZE) {
    *SignalCollector::ringWrite++ = t;
    buf88_cnt++;

    // reset pointer to the start when reaching end
    if (SignalCollector::ringWrite == SignalCollector::buf88_end)
      SignalCollector::ringWrite = SignalCollector::buf88;
  } // if

  SignalCollector::lastTime = micros();
} // injectTiming()


int SignalCollector::_recvPin = -1;

// allocate and initialize the static class members.

unsigned long SignalCollector::lastTime = 0;

// allocate memory for ring buffer
SignalParser::CodeTime *SignalCollector::buf88 = (SignalParser::CodeTime *)malloc(SC_BUFFERSIZE * sizeof(SignalParser::CodeTime));

// write pointer starts at start
volatile SignalParser::CodeTime *SignalCollector::ringWrite = SignalCollector::buf88;

// read pointer starts at start
volatile SignalParser::CodeTime *SignalCollector::buf88_read = SignalCollector::buf88;

// end of buffer + 1 pointer for wrapping
SignalParser::CodeTime *SignalCollector::buf88_end = SignalCollector::buf88 + SC_BUFFERSIZE;

volatile unsigned int SignalCollector::buf88_cnt = 0; // number of bytes in buffer

// End.
