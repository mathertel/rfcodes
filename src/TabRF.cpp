

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

// bit 0      : s = start
// bit 1..26  : unique ID of the transmitter device
// bit 27     : group flag, if set, all reivers with the same id must act
// bit 28     : 0= OFF, 1=ON, D=dimming
// bit 29..32 : unit number
// bit 33..36 : dim level
// x

// <v:len:0:1>
// $v:4:0:1

// s<id:26>GV<unit:4><level:4>x    
// s**************************GV####x
// s_##___#____#_#__###_____#__#____x    on
// s_##___#____#_#__###_____#_______x    off
// s_##___#____#_#__###_____#__D__##_#__x

// http://rfdevice/$board/rfsend/i1?value=0
// http://rfdevice/$board/rfsend/i1?value=1

void TabRFClass::send(const char *signal, int value)
{
  String fullSignal = signal;

  Code *thisCode;
  int level = LOW;

  if ((_sendPin >= 0) && (_sequence)) {
    if (_recvPin >= 0)
      detachInterrupt(_irq);

    int8_t repeat = _sequence->sendRepeat;
    while (repeat > 0) {
      repeat--;

      const char *c = signal;
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
    if (_recvPin >= 0)
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
  if (recvPin >= 0) {
    _irq =
        digitalPinToInterrupt(recvPin); // will return -1 on wrong pin number.
    if (_irq >= 0) {
      pinMode(_recvPin, INPUT);
      _resetCode(true);
      attachInterrupt(_irq, signal_change_handler, CHANGE);
    } // if
  }

  _sendPin = sendPin;
  if (sendPin >= 0) {
    // initialize sending mode
    pinMode(_sendPin, OUTPUT);
  }
} // init()


// ===== the one and only tabRF object. =====
TabRFClass tabRF;

// End.
