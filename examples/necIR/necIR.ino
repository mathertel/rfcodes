/*
  file: nectIR.ino
  This sample is part of the TabRF library showing how to receive and send
  Infrared codes.
  
  This example shows how the nec IR protocol can be decoded with a IR receiver.

  http://www.mathertel.de/Arduino/TabRFlibrary.aspx

  Use the Serial Monitor to see the received codes and send the predefined
  codes.

  05.06.2020 created
*/

#include <Arduino.h>
#include <SignalCollector.h>
#include <nec_ir_protocol.h>

#include <SignalParser.h>


bool showRaw = false;

SignalParser sig;
TabRFClass tabRF;

// ===== Cresta protocol decoding =====

// This function can be used to decode the Cresta Manchester protocol.
// See cresta.h for further details.

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
  Serial.println("necIR Packet Receiver");
  delay(2000);
  Serial.println();

  // initialize the tabRF library
  tabRF.init(&sig, D7, NO_PIN, 16); // input at pin D7, no output

  register_nec_ir(sig);

  sig.dumpTable();

  if (showRaw)
    Serial.println("Raw mode is enabled");
  else
    Serial.println("Raw mode is disabled");

  sig.attachCallback(receiveCode);
} // setup()


unsigned int maxBufCount = 0;

void loop()
{
  unsigned long now = millis();
  static unsigned long nextReport = now + 1000;

  if (Serial.available() > 0) {
    char cmd = Serial.read();
    if (cmd == '1') {
      Serial.println("Sending (C1)...");

    } else if (cmd == '2') {
      Serial.println("Sending (C2)...");

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
