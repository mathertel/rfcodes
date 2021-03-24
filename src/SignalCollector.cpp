// @file TabRF.cpp

#include "TabRF.h"

// ====== TabRFClass implemenation =====

/**
 * Initialize the receiving and sending modes and activate the IO pins
 * @param recvPin The IO pin to be used for receiving. Set to -1 to disable
 * receiving mode.
 * @param sendPin The IO pin to be used for sending. Set to -1 to disable
 * sending mode.
 */
void TabRFClass::init(SignalParser *sig, int recvPin, int sendPin, int trim)
{
  TRACE_MSG("Initalizing tabRF hardware\n");

  _sig = sig;
  _trim = trim;

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


void TabRFClass::send(const char *signal, int value)
{
  SignalParser::CodeTime timings[MAX_TIMING_LENGTH];
  int level = LOW; // LOW level before starting.
  INFO_MSG("send(%s)", signal);

  char protname[PROTNAME_LEN];
  _sig->strcpyProtname(protname, (char *)signal);
  INFO_MSG("send prot %s", protname);

  int repeat = _sig->getSendRepeat(protname);
  INFO_MSG("send repeat %d", repeat);

  if ((repeat) && (_sendPin >= 0)) {
    if (_recvPin >= 0)
      detachInterrupt(_irNumber);

    // get timings of the code
    _sig->compose(signal, timings, sizeof(timings));
    // dumpTimings(timings);

    while (repeat) {
      SignalParser::CodeTime *t = timings;

      while (*t) {
        level = !level;
        digitalWrite(_sendPin, level);
        delayMicroseconds(*t++);
      } // while
      repeat--;
    } // while

    if (_recvPin >= 0)
      attachInterrupt(_irNumber, signal_change_handler, CHANGE);
  } // if
} // send()


// process bytes from ring buffer
void TabRFClass::loop()
{
  while (TabRFClass::buf88_cnt > 0) {
    SignalParser::CodeTime t = *TabRFClass::buf88_read++;
    TabRFClass::buf88_cnt--;

    _sig->parse(t);

    // reset pointer to the start when reaching end
    if (TabRFClass::buf88_read == TabRFClass::buf88_end)
      TabRFClass::buf88_read = TabRFClass::buf88;
    yield();
  } // while
} // loop


// ===== Insights and Debugging Helpers =====


/** Return the last received timings from the ring-buffer. */
void TabRFClass::getBufferData(SignalParser::CodeTime *buffer, int len)
{
  len--; // keep space for final '0';
  if (len > buf88_size)
    len = buf88_size;

  SignalParser::CodeTime *p = (SignalParser::CodeTime *)buf88_read - len;
  if (p < buf88)
    p += buf88_size;

  // copy timings to buffer
  while (len) {
    *buffer++ = *p++;

    // reset pointer to the start when reaching end
    if (p == TabRFClass::buf88_end)
      p = TabRFClass::buf88;
    len--;
  } // if
  *buffer = 0;
}; // getBufferData()


/** dump the data from a table of timings that end with a 0 time. */
void TabRFClass::dumpTimings(SignalParser::CodeTime *raw)
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
void ICACHE_RAM_ATTR TabRFClass::signal_change_handler()
{
  // last time the interrupt was called.
  volatile static unsigned long lastTime = micros();

  unsigned long now = micros();
  SignalParser::CodeTime t = (SignalParser::CodeTime)(now - lastTime);

  // adjust the timing with the trim factor.
  int level = digitalRead(_recvPin);
  if (level) {
    t += TabRFClass::_trim; // end of low
  } else {
    t -= TabRFClass::_trim; // end of high
  }

  // write to ring buffer
  if (TabRFClass::buf88_cnt < buf88_size) {
    *TabRFClass::buf88_write++ = t;
    buf88_cnt++;

    // reset pointer to the start when reaching end
    if (TabRFClass::buf88_write == TabRFClass::buf88_end)
      TabRFClass::buf88_write = TabRFClass::buf88;
  } // if

  lastTime = now; // micros();
} // signal_change_handler()


int TabRFClass::_recvPin = -1;
int TabRFClass::_trim = 0;

SignalParser::CodeTime *TabRFClass::buf88 =
    (SignalParser::CodeTime *)malloc(buf88_size * sizeof(SignalParser::CodeTime)); // allocated memory
volatile SignalParser::CodeTime *TabRFClass::buf88_write =
    TabRFClass::buf88; // write pointer
volatile SignalParser::CodeTime *TabRFClass::buf88_read = TabRFClass::buf88; // read pointer
SignalParser::CodeTime *TabRFClass::buf88_end =
    TabRFClass::buf88 + buf88_size; // end of buffer+1 pointer for wrapping
volatile unsigned int TabRFClass::buf88_cnt = 0; // number of bytes in buffer

// End.
