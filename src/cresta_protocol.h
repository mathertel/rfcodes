// cresta.h

// These protocol definitions support manchester encoded protocol
// that is used in some temperature and humidity sensors using 433 MHz transmission.
// These sensors have a fixed starting byte that is transferred using 5 long timings in a row.
// The following timings are either one long (l) or two short timings (s) as defined in the manchester protocol.
// See https://en.wikipedia.org/wiki/Manchester_code how the manchester code is built.
// The recorded length is 7 bytes.
// More details can be seen in http://members.upc.nl/m.beukelaar/Crestaprotocol.pdf.

// More on the Manchester and Cresta protocol in /docs/cresta_protocol.md
// To decode the temperature and humidity see example `TempSensor.ino`.

#ifndef SignalParser_CRESTA_H_
#define SignalParser_CRESTA_H_

#include "SignalParser.h"


/** register the "older" intertechno protocol */
void register_cresta(SignalParser &sig)
{
  // the base code timing is according to 1kHz sending frequence ~ 500µsecs
  SignalParser::CodeTime t = 500;

  // This protocol has a fixed length of 59 codes for 7 bytes: 9*7-4
  uint8_t prot = sig.newProtocol("cw", 59, 59, 25);

  // starting sequence is 5 times long timings
  sig.newCode(prot, 'H', SP_START, 2 * t, 2 * t, 2 * t, 2 * t, 2 * t);

  // manchester short timings always come in pairs
  sig.newCode(prot, 's', SP_DATA, t, t);

  // manchester long timings
  sig.newCode(prot, 'l', SP_DATA, 2 * t);
} // register_cresta()

#endif // SignalParser_CRESTA_H_

// End.
