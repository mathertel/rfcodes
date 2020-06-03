/*
  file: intertechno2.ino
  This sample is part of the TabRF library showing how to receive and send
  intertechno2 compatible codes.
  
  This example shows that 2 different protocols can be decoded with the same receiver
  by registering 2 protocols used by the intertechno devices.

  http://www.mathertel.de/Arduino/TabRFlibrary.aspx

  Use the Serial Monitor to see the received codes and send the predefined
  codes.

  25.03.2018 ported to ESP8266
  29.03.2018 converted to library
*/

#include <Arduino.h>
#include <TabRF.h>
#include <cresta_protocol.h>
#include <intertechno_protocol.h>
#include <sc5272_protocol.h>
#include <nec_ir_protocol.h>

#include <signal_parser.h>


bool showRaw = false;

SignalParser sig;
TabRFClass tabRF;

// ===== Cresta protocol decoding =====

// This function can be used to decode the Cresta Manchester protocol.
// See cresta.h for further details.

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

  // analysing supporting callback
  if (showRaw && (*proto == '*')) {
    int n = atoi(proto + 1) + 1;
    tabRF.getBufferData(lastProbes, min(n, 150));

    // dump probes
    SignalParser::CodeTime *p = lastProbes;
    int len = 0;
    while (len < 150 && *p) {
      if (len % 8 == 0) {
        Serial.printf("%3d: %5u,", len, *p);
      } else if (len % 8 == 7) {
        Serial.printf(" %5u,\n", *p);
      } else {
        Serial.printf(" %5u,", *p);
      }
      p++;
      len++;
    } // while
    Serial.println();
  } // if

  if (strncmp(proto, "cw ", 3) == 0) {
    cresta_decode(proto + 4);
  }
} // receiveCode()

void setup()
{
  Serial.begin(115200);
  Serial.println("TabRF Packet Receiver");
  delay(2000);
  Serial.println();

  Serial.println(
      "Commands: 1-2(Send Code) T(Dump Code Table) R(aw toggle)");

  // initialize the tabRF library
  tabRF.init(&sig, D7, NO_PIN, 16); // input at pin D8 , output a pin D3

  // register_intertechno1(sig);
  // register_intertechno2(sig);
  register_cresta(sig);
  register_SC5272(sig, 95);
  register_nec_ir(sig);

  sig.dumpTable();
  if (showRaw)
    Serial.println("Raw mode is enabled");
  else
    Serial.println("Raw mode is disabled");

  sig.attachCallback(receiveCode);
} // setup()


#define C1 "it2 s_##___#____#_#__###_____#__#_#__x"
#define C2 "it2 s_##___#____#_#__###_____#____#__x"

unsigned int maxBufCount = 0;

void loop()
{
  unsigned long now = millis();
  static unsigned long nextReport = now + 1000;

  if (Serial.available() > 0) {
    char cmd = Serial.read();
    if (cmd == '1') {
      Serial.println("Sending (C1)...");
      tabRF.send(C1);

    } else if (cmd == '2') {
      Serial.println("Sending (C2)...");
      tabRF.send(C2);

    } else if (cmd == 'T') {
      sig.dumpTable();
      if (showRaw)
        Serial.println("Raw mode is enabled");
      else
        Serial.println("Raw mode is disabled");

    } else if (cmd == 'R') {
      showRaw = !showRaw;

    } // if
  } // if

  SignalParser::CodeTime bufcnt = tabRF.getBufferCount();
  if ((now > nextReport) && (bufcnt > 40)) {
    Serial.print("buf_cnt:");
    Serial.println(bufcnt);
    nextReport = now + 1000;
  }

  // process received bytes
  tabRF.loop();
} // loop()
