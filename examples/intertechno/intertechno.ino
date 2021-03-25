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
#include <SignalCollector.h>
// #include <cresta_protocol.h>
// #include <sc5272_protocol.h>

#include <SignalParser.h>

#include <protocols.h>

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
      "Commands: 1-2(Send Code) D(ump Code Table) T(est data) R(aw toggle)");

  // initialize the tabRF library
  tabRF.init(&sig, D7, NO_PIN, 16); // input at pin D7, no output

  // register_intertechno1(sig);
  // register_intertechno2(sig);
  // register_cresta(sig);
  // register_SC5272(sig, 95);

  sig.load(&it1);
  sig.load(&it2);

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

SignalParser::CodeTime testdata[] = {
    // noise
    13462, 70, 1433, 171, 232, 98, 1239, 337, 318, 52, 469, 182, 340, 432, 2860, 269, 4056, 108, 3290, 79, 2904, 260, 2870, 158, 7818, 75, 2047, 183, 520, 152, 161, 115, 114, 329, 340, 95, 4309, 153, 5210, 28, 2966, 273, 4856, 75, 955, 289, 333, 254, 433, 65,
    52, 44, 447, 346, 3852, 120, 1086, 287, 366, 123, 53, 153, 10, 14, 660, 96, 5486, 97, 3526, 398, 361, 325, 51, 188, 1983, 131, 614, 7, 1902, 2783, 4368, 2536, 22, 323, 1381, 312, 3249, 249, 1397, 366, 42182, 101, 33860, 286, 649, 629, 14476, 129, 261, 609,

    // valid it1: [it1 B000110000001]
    443, 13281,
    460, 1257, 1313, 403, 464, 1254, 1310, 412, 450, 1263, 1311, 410, 449, 1265, 449, 1269, 452, 1265, 454, 1265, 446, 1267, 1312, 408, 452, 1264, 1310, 409, 451, 1265, 1311, 410, 448, 1267, 1305, 417, 443, 1271, 1306, 411, 445, 1274, 1308, 410, 443, 1276, 442, 1271,

    // noise
    70, 1433, 171, 232, 98, 1239, 337, 318, 52, 955, 289, 333, 254, 433, 65,

    // // valid it2: [it2 s_##__##__#__####____##__#_______x]
    327, 2760, 326, 246, 324, 1316, 332, 1312, 320, 249, 332, 1307, 323, 257, 328, 237, 329, 1318, 327, 239, 323, 1324, 323, 1309, 336, 246, 327, 1304, 336, 256, 313, 243, 327, 1331, 324, 246, 323, 1322, 322, 1314, 324, 254, 326, 242, 324, 1317, 330, 245, 323, 1323,
    322, 1308, 325, 252, 323, 1315, 324, 258, 316, 1316, 328, 253, 320, 1313, 325, 262, 326, 244, 329, 1316, 325, 242, 330, 1322, 322, 240, 325, 1320, 326, 244, 325, 1328, 316, 1311, 330, 247, 326, 1310, 324, 257, 319, 251, 321, 1326, 318, 254, 315, 1340, 319, 1312,
    319, 265, 320, 243, 320, 1321, 325, 246, 323, 1321, 325, 247, 317, 1329, 314, 254, 321, 1320, 324, 249, 316, 1329, 318, 249, 319, 1328, 320, 249, 314, 1318, 321, 8387,

    // noise
    70, 1433, 171, 232, 98, 1239, 337, 318, 52, 955, 289, 333, 254, 433, 65,

    // tricky it2: [it2 s_##__##__#__####____##__#______#x]
    305,
    327, 2760, 326, 246, 324, 1316, 332, 1312, 320, 249, 332, 1307, 323, 257, 328, 237, 329, 1318, 327, 239, 323, 1324, 323, 1309, 336, 246, 327, 1304, 336, 256, 313, 243, 327, 1331, 324, 246, 323, 1322, 322, 1314, 324, 254, 326, 242, 324, 1317, 330, 245, 323, 1323,
    322, 1308, 325, 252, 323, 1315, 324, 258, 316, 1316, 328, 253, 320, 1313, 325, 262, 326, 244, 329, 1316, 325, 242, 330, 1322, 322, 240, 325, 1320, 326, 244, 325, 1328, 316, 1311, 330, 247, 326, 1310, 324, 257, 319, 251, 321, 1326, 318, 254, 315, 1340, 319, 1312,
    319, 265, 320, 243, 320, 1321, 325, 246, 323, 1321, 325, 247, 317, 1329, 314, 254, 321, 1320, 324, 249, 316, 1329, 318, 249, 319, 1328, 314, 1318, 320, 249, 321, 8387,

    // noise
    337,
    10857, 301, 251, 463, 237, 139, 4313, 124, 14818, 451, 43, 838, 89, 205, 15003, 171, 960, 636, 9747, 2635, 1745, 334,
    2478, 2497, 19689, 94, 40587, 168, 14091, 98, 4104, 230, 996, 672, 13955, 185, 5048, 651, 12689, 404, 654, 143, 823, 122, 225, 190, 2199, 242, 1142, 65, 226, 208, 105, 29, 479, 350, 4207, 226, 162, 219, 96, 126, 4207, 398, 5610, 28, 4426, 34, 1144, 170, 1753, 23,
    3314,

    0};

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
      Serial.println("Sending Test Data...");
      sig.test(testdata);

    } else if (cmd == 'D') {
      sig.dumpTable();
      if (showRaw)
        Serial.println("Raw mode is enabled");
      else
        Serial.println("Raw mode is disabled");

    } else if (cmd == 'R') {
      showRaw = !showRaw;
      if (showRaw)
        Serial.println("Raw mode is enabled");
      else
        Serial.println("Raw mode is disabled");

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
