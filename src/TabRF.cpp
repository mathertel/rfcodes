

#include "TabRF.h"
#include "Arduino.h"

// ====== TabRFClass implemenation =====

TabRFClass::TabRFClass()
{
  DEBUG_MSG("TabRFClass created.\n");
}


void TabRFClass::attachCallback(TabRFCallbackFunction newFunction)
{
  _callbackFunc = newFunction;
} // attachCallback()


void TabRFClass::send(char *code)
{
  Code *thisCode;
  int level = LOW;

  if (_sequence) {
    detachInterrupt(_irq);

    int8_t repeat = _sequence->sendRepeat;
    while (repeat > 0) {
      repeat--;

      char *c = code;
      while (*c) {
        // search the code in the table
        for (int8_t n = 0; n < _sequence->length; n++) {
          if (_sequence->codes[n].name == *c) {
            thisCode = &_sequence->codes[n];
            for (int8_t i = 0; i < thisCode->length; i++) {
              level = !level;
              digitalWrite(_sendPin, level);
              delayMicroseconds(thisCode->time[i]);
            } // for
          } // if
        } // for
        c++;
      } // while
    } // for

    // reset current protocol and enable receiving again.
    _resetCode(true);
    attachInterrupt(_irq, signal_change_handler, CHANGE);
  } // if
} // send()


void TabRFClass::init(int recvPin, int sendPin)
{
  DEBUG_MSG("Initalizing tabRF hardware\n");
  _sequence = NULL;

  // initialize receiving mode with interrupt service routine
  // See
  // https://www.arduino.cc/reference/en/language/functions/external-interrupts/attachinterrupt/
  _recvPin = recvPin;
  _irq = digitalPinToInterrupt(recvPin); // will return -1 on wrong pin number.
  if (_irq >= 0) {
    pinMode(_recvPin, INPUT);
    _resetCode(true);
    attachInterrupt(_irq, signal_change_handler, CHANGE);
  } // if

  // initialize sending mode
  _sendPin = sendPin;
  pinMode(_sendPin, OUTPUT);
} // init()


// ===== the one and only tabRF object. =====
TabRFClass tabRF;

// End.
