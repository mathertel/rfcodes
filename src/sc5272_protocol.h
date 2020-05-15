// intertechno.h

// These protocol definitions support the 2 generatzions of intertechno devices
// The "older" protocol with 12 data bits can be activated by `register_intertechno1`
// The "newer" protocol with 32 data bits can be activated by `register_intertechno2`


#ifndef SC5272_PROTOCOL_H_
#define SC5272_PROTOCOL_H_


#include "signal_parser.h"


void register_SC5272(SignalParser &sig, SignalParser::CodeTime t = 100) {

  // this protocol has a fixed length of 12 codes
  uint8_t prot = sig.newProtocol("sc5", 1 + 12, 1 + 12, 15);

  // low signal is /‾\___/‾‾‾\_/
  sig.newCode(prot, '0', (SP_START | SP_DATA), 4 * t, 12 * t, 4 * t, 12 * t);

  // high signal is /‾‾‾\_/‾‾‾\_/
  sig.newCode(prot, '1', (SP_START | SP_DATA), 12 * t, 4 * t, 12 * t, 4 * t);

  // float signal is /‾\___/‾\___/
  sig.newCode(prot, 'f', (SP_START | SP_DATA), 4 * t, 12 * t, 12 * t, 4 * t);

  // starting sequence is /‾\___(long)_______/
  sig.newCode(prot, 'S', SP_END, 4 * t, 124 * t);
}  // register_SC5272()

#endif  // SC5272_PROTOCOL_H_

// End.
