// nec_ir_protocol.h

// https://www.sbprojects.net/knowledge/ir/nec.php
// https://techdocs.altium.com/display/FPGA/NEC+Infrared+Transmission+Protocol


#ifndef SIGNAL_PARSER_NEC_IR_H_
#define SIGNAL_PARSER_NEC_IR_H_

#include "signal_parser.h"


/** register the "older" intertechno protocol */
void register_nec_ir(SignalParser &sig) {
  // this protocol has a fixed length of 12 codes
  uint8_t prot = sig.newProtocol("nec", 1, 1 + 32, 20);

  uint32_t t = 560; 

  // starting sequence is /‾\___(long)_______/
  sig.newCode(prot, 'N', SP_START, 9000, 4500);

  // low signal is /‾\_/
  sig.newCode(prot, '0', SP_DATA, t, t);

  // high signal is /‾\___/
  sig.newCode(prot, '1', SP_DATA, t, 3 * t);

  // Repeat signal is /‾\___/
  sig.newCode(prot, 'R', SP_START | SP_END, 9000, 2250, t);



  // this protocol has a fixed length of 12 codes
  prot = sig.newProtocol("necR", 1, 1, 20);

  t = 560;

  // Repeat signal is /‾\___/
  sig.newCode(prot, 'X', SP_START | SP_END, 9000, 2250, t);

}  // register_nec_ir()

#endif  // SIGNAL_PARSER_NEC_IR_H_

// End.
