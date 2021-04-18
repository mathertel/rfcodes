/**
 * @file intertechno.ino
 * 
 * @author Matthias Hertel (https://www.mathertel.de)
 * @copyright Copyright (c) by Matthias Hertel, https://www.mathertel.de.
 * This work is licensed under a BSD 3-Clause style license, see https://www.mathertel.de/License.aspx

 * @brief Receive and send codes using intertechno protocols.
 * This file is part of the RFCodes library that implements receiving an sending RF and IR protocols.
 *
 * This example shows how to receive and send codes from the intertechno devices.
 * The 2 different protocols used by this vendor can be decoded with the same receiver
 * by registering the 2 protocols.
 *
 * Some codes are defined in this sketch and can be sent out.
 *
 * Wiring (ESP8266):
 * * a receiver can be attached with data to pin D5.
 * * a transmitter can be attached with data to pin D6.
 *
 * Use the Serial Monitor to see the received codes and send the predefined codes.
*/

#include <Arduino.h>
#include <SignalCollector.h>
#include <SignalParser.h>

#include <protocols.h>

bool showRaw = false;

SignalParser sig;
SignalCollector col;

// ===== Cresta protocol decoding =====

// This function can be used to decode the Cresta Manchester protocol.
// See /docs/cresta_potocol.md for further details.

void cresta_decode(const char *p)
{
  uint8_t cresta_data[10]; // our device emits 10+2 data bytes, we only read the 10 and ignore the checksum
  int cresta_cnt = 0;      // number of received bytes

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
void receiveCode(const char *proto)
{
  SignalParser::CodeTime lastProbes[120 + 1]; // dividable by 8 is preferred.
  Serial.printf("received [%s]\n", proto);

  // analysing supporting callback
  if (showRaw) {
    col.getBufferData(lastProbes, 120 + 1);

    // dump probes
    SignalParser::CodeTime *p = lastProbes;
    int len = 0;
    while (len < 121 && *p) {
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
  delay(2000);
  Serial.begin(115200);
  Serial.println("RFCodes Intertechno Example");
  Serial.println();

  Serial.println(
      "Commands: 1-2(Send Code) D(ump Code Table) R(aw toggle)");

  // load the protocols into the SignalParser
  sig.load(&RFCodes::it1);
  sig.load(&RFCodes::it2);
  sig.load(&RFCodes::sc5);
  sig.load(&RFCodes::cw);

  // show all defined protocols
  sig.dumpTable();

  // initialize the SignalCollector library
  col.init(&sig, D5, D6); // input at pin D5, output at pin D6

  if (showRaw)
    Serial.println("Raw mode is enabled");
  else
    Serial.println("Raw mode is disabled");

  sig.attachCallback(receiveCode);
} // setup()


#define C1 "it2 s_##___#____#_#__###_____#__#_#__x"
#define C2 "it2 s_##___#____#_#__###_____#____#__x"
#define C3 "it1 B111101111000"
#define C4 "it1 B111101111001"

void loop()
{
  unsigned long now = millis();
  static unsigned long nextReport = now + 1000;

  if (Serial.available() > 0) {
    char cmd = Serial.read();
    if (cmd == '1') {
      Serial.println("Sending (C1)...");
      col.send(C1);

    } else if (cmd == '2') {
      Serial.println("Sending (C2)...");
      col.send(C2);

    } else if (cmd == '3') {
      Serial.println("Sending (C3)...");
      col.send(C3);

    } else if (cmd == '4') {
      Serial.println("Sending (C4)...");
      col.send(C3);

    } else if (cmd == 'R') {
      showRaw = !showRaw;
      if (showRaw)
        Serial.println("Raw mode is enabled");
      else
        Serial.println("Raw mode is disabled");

    } else if (cmd == 'D') {
      sig.dumpTable();

    } // if
  }   // if


  SignalParser::CodeTime bufcnt = col.getBufferCount();
  if ((now > nextReport) && (bufcnt > 40)) {
    Serial.print("buf_cnt:");
    Serial.println(bufcnt);
    nextReport = now + 1000;
  }

  // process received bytes
  col.loop();
} // loop()
