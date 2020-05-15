/*
  file: TempSensor.ino
  This example is part of the TabRF library showing how to receive codes from a temperature and humidity sensor using a 433 MHz protocol.
  For more details see cresta_protocol.md in the docu folder. 

  13.05.2020 created from receiver.ino
*/

#include <Arduino.h>
#include <TabRF.h>
#include <cresta.h>

SignalParser sig;
TabRFClass tabRF;

// ===== Cresta protocol decoding =====

// This function can be used to decode the Cresta Manchester protocol.

void cresta_decode(const char *p)
{
  uint8_t cresta_data[10]; // our device emits 10+2 data bytes, we only read the 10 and ignore the checksum
  int cresta_cnt = 0; // number of received bytes

  uint8_t cresta_byte = 0; // currenty byte value fom the stream
  uint8_t cresta_bits = 0; // next bit to receive. 0..7, 8 is the 0-bit used inbetween data baytes

  // simulate shifting in bits from header : 10101
  cresta_byte = 0x15;
  cresta_bits = 5;
  bool bit = 1; // last bit was HIGH

  while (*p) {
    if (*p == 'l') {
      bit = 1 - bit;
    }
    p++;

    if (cresta_bits < 8) {
      // shift bit into data byte
      cresta_byte |= bit << cresta_bits;
      cresta_bits++;
    } else {
      // shift completed byte into buffer
      // bit must be 0, not verified
      cresta_byte = cresta_byte ^ (cresta_byte << 1); // decode
      cresta_data[cresta_cnt++] = cresta_byte;
      // reset for next byte
      cresta_bits = 0;
      cresta_byte = 0;
    }
  } // while

  // print received data
  Serial.print("data:");
  for (int i = 0; i < 7; i++) {
    Serial.printf("%02x ", cresta_data[i]);
  } // for
  Serial.println();

  if (cresta_data[0] != 0x9f) {
    Serial.println("Bad data.");
  } else {
    // temperature
    int temp = 100 * (cresta_data[5] & 0x0f) + 10 * (cresta_data[4] >> 4) + (cresta_data[4] & 0x0f);
    Serial.printf("  temp: %d.%d °C\n", temp / 10, temp % 10);

    // humidity
    int hum = 10 * (cresta_data[6] >> 4) + (cresta_data[6] & 0x0f);
    Serial.printf("  hum : %d %%\n", hum);
  }
} // cresta_decode


// This function will be called when a complete protcol was received.
void receiveCode(char *proto)
{
  SignalParser::CodeTime lastProbes[152]; // dividable by 8 is preferred.
  // remember last code in a local variable
  Serial.printf("[%s]\n", proto);

  if (strncmp(proto, "cw ", 3) == 0) {
    cresta_decode(proto + 4);
  }
} // receiveCode()


void setup()
{
  Serial.begin(115200);
  Serial.println("Cresta Protocol Receiver");
  Serial.println();

  // initialize the tabRF library with ESP8266 pin D7 for receiving
  tabRF.init(&sig, D7, NO_PIN, 16); // input at pin D7, no output

  // register the cresta protocol
  register_cresta(sig);

  sig.dumpTable();
  sig.attachCallback(receiveCode);
} // setup()


void loop()
{
  // process received bytes
  tabRF.loop();
} // loop()

// End.
