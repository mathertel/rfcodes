// intertechno.h

// These protocol definitions support the 2 generatzions of intertechno devices
// The "older" protocol with 12 data bits can be activated by `register_intertechno1`
// The "newer" protocol with 32 data bits can be activated by `register_intertechno2`


#ifndef SIGNAL_PARSER_INTERTECHNO_H_
#define SIGNAL_PARSER_INTERTECHNO_H_

#include "signal_parser.h"


/** register the "older" intertechno protocol */
void register_intertechno1(SignalParser &sig) {
  // this protocol has a fixed length of 12 codes
  uint8_t prot = sig.newProtocol("it1", 1 + 12, 1 + 12, 20);

  uint32_t t = 380; // 95*4

  // starting sequence is /‾\___(long)_______/
  sig.newCode(prot, 'B', SP_START, t, 31 * t);

  // low signal is /‾\___/‾‾‾\_/
  sig.newCode(prot, '0', SP_DATA, t, 3 * t, 3 * t, t);

  // high signal is /‾\___/‾\___/
  sig.newCode(prot, '1', SP_DATA, t, 3 * t, t, 3 * t);
}  // register_intertechno1()



/** register the "newer" intertechno protocol */
void register_intertechno2(SignalParser &sig) {
  uint32_t a = 280;

  uint8_t prot = sig.newProtocol("it2", 34, 48, 25, 10);

  // starting sequence is /‾\__________/
  sig.newCode(prot, 's', SP_START, a, 10 * a);

  // low signal is /‾\_/‾\_____/
  sig.newCode(prot, '_', SP_DATA, a, a, a, 5 * a);

  // high signal is /‾\_____/‾\_/
  sig.newCode(prot, '#', SP_DATA, a, 5 * a, a, a);

  // DIM signal is /‾\_/‾\_/  (shorter !)
  sig.newCode(prot, 'D', SP_DATA, a, a, a, a);

  // end signal is /‾\_/‾\______40 times____/  (longer !)
  sig.newCode(prot, 'x', SP_END, a, 10168);
}  // register_intertechno2()


#endif  // SIGNAL_PARSER_INTERTECHNO_H_

// End.
