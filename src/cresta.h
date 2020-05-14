// cresta.h

// These protocol definitions support manchester encoded protocol
// that is used in some temperature and humidity sensors using 433 MHz transmission.
// These sensors have a fixed starting byte that is transferred using 5 long timings in a row.
// The following timings are either one long (l) or two short timings (s) as defined in the manchester protocol.
// See https://en.wikipedia.org/wiki/Manchester_code 
// The recorded length is 7 bytes.
// More details can be seen in http://members.upc.nl/m.beukelaar/Crestaprotocol.pdf.
// To decode the temperature and humidity see example `sensor.ino`. 

#ifndef SIGNAL_PARSER_CRESTA_H_
#define SIGNAL_PARSER_CRESTA_H_

#include "signal_parser.h"


/** register the "older" intertechno protocol */
void register_cresta(SignalParser &sig)
{
  // this protocol has a fixed length of 76 codes: 9*7-4
  uint32_t b = 500;
  uint8_t prot = sig.newProtocol("cw", 59, 59, 25);

  // starting sequence is 3 times long timings
  sig.newCode(prot, 'H', SP_START, 2 * b, 2 * b, 2 * b, 2 * b, 2 * b);

  // manchester short timings always come in pairs
  sig.newCode(prot, 's', SP_DATA, b, b);

  // manchester long timings
  sig.newCode(prot, 'l', SP_DATA, 2 * b);
} // register_cresta()

#endif // SIGNAL_PARSER_CRESTA_H_

// End.
