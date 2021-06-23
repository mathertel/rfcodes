/**
 * @file TempSensor.ino
 * 
 * @author Matthias Hertel (https://www.mathertel.de)
 * @copyright Copyright (c) by Matthias Hertel, https://www.mathertel.de.
 * This work is licensed under a BSD 3-Clause style license, see https://www.mathertel.de/License.aspx
 *
 * @brief Receive codes from a temperature and humidity sensor.
 * This file is part of the RFCodes library that implements receiving an sending RF and IR protocols.
 *
 * This example shows how to use a 433 MHz receiver to get and decode a protocol from a temperature and humidity sensor.
 * For more details see cresta_protocol.md in the docu folder.
 * 
 * Wiring (ESP8266):
 * * a receiver can be attached with data to pin D5.

 * * 13.05.2020 created from receiver.ino
 */

#include <Arduino.h>
#include <SignalCollector.h>
#include <SignalParser.h>

#include <protocols.h>

SignalParser sig;
SignalCollector col;

// ===== Cresta protocol decoding =====

// This function can be used to decode the Cresta Manchester protocol.

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
    if (cresta_data[i] < 16) Serial.print("0");
    Serial.print(cresta_data[i], 16);
    Serial.print(" ");
  } // for
  Serial.println();

  if (cresta_data[0] != 0x9f) {
    Serial.println("Bad data.");
  } else {
    // temperature
    int temp = 100 * (cresta_data[5] & 0x0f) + 10 * (cresta_data[4] >> 4) + (cresta_data[4] & 0x0f);
    Serial.print("  temp: ");
    Serial.print(temp / 10);
    Serial.print('.');
    Serial.print(temp % 10);
    Serial.println(" °C");

    // humidity
    int hum = 10 * (cresta_data[6] >> 4) + (cresta_data[6] & 0x0f);
    Serial.print("   hum: ");
    Serial.print(hum);
    Serial.println(" %");
  }
} // cresta_decode


// This function will be called when a complete protcol was received.
void receiveCode(const char *proto)
{
  Serial.print("received [");
  Serial.print(proto);
  Serial.println("]");

  if (strncmp(proto, "cw ", 3) == 0) {
    cresta_decode(proto + 4);
  }
} // receiveCode()


void setup()
{
  Serial.begin(115200);
  Serial.println("Cresta Protocol Receiver Example");
  Serial.println();

    // load the protocol into the SignalParser
  sig.load(&RFCodes::cw);

  // show all defined protocols
  sig.dumpTable();

  // initialize the SignalCollector library
  col.init(&sig, D5, NO_PIN); // input at pin D5, no output

  sig.attachCallback(receiveCode);
} // setup()


void loop()
{
  // process received bytes
  col.loop();
} // loop()

// End.
