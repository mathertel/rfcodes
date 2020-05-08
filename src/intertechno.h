// intertechno.h

// 

#ifndef SIGNAL_PARSER_INTERTECHNO_H_
#define SIGNAL_PARSER_INTERTECHNO_H_

#include "signal_parser.h"

// https://github.com/pimatic/rfcontroljs/blob/master/src/protocols/switch4.coffee

void register_intertechno1(SignalParser &sig)
{
  // this protocol has a fixed length of 12 codes
  uint32_t b = 420;
  uint8_t prot = sig.newProtocol("it1", 1+12, 1+12, 20);

  // starting sequence is /‾\___(long)_______/
  sig.newCode(prot, 'B', SP_START, b, 31 * b);
  // low signal is /‾\___/‾‾‾\_/
  sig.newCode(prot, '0', SP_DATA, b, 3 * b, 3 * b, b);
  // high signal is /‾\___/‾\___/
  sig.newCode(prot, '1', SP_DATA, b, 3 * b, b, 3 * b);
} // register_intertechno1()


void register_intertechno2(SignalParser &sig)
{
  // examples:
  // [it2 s_##__##__#__####____##__#_______x]
  // [it2 s_##__##__#__####____##__#______#x]
  uint8_t prot = sig.newProtocol("it2", 34, 48, 35);

  // starting sequence is /‾\__________/
  sig.newCode(prot, 's', SP_START, 286, 2632);

  // low signal is /‾\_/‾\_____/
  sig.newCode(prot, '_', SP_DATA, 286, 286, 286, 1300);

  // high signal is /‾\_____/‾\_/
  sig.newCode(prot, '#', SP_DATA, 286, 1300, 286, 286);

  // DIM signal is /‾\_/‾\_/  (shorter !)
  sig.newCode(prot, 'D', SP_DATA, 286, 286, 286, 286);

  // end signal is /‾\_/‾\______40 times____/  (longer !)
  sig.newCode(prot, 'x', SP_END, 286, 10168);
} // register_intertechno2()


#endif // SIGNAL_PARSER_INTERTECHNO_H_
