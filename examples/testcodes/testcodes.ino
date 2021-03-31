/**
 * @file: testcodes.ino
 * 
 * This file is part of the RFCodes library that implements receiving an sending
 * RF and IR protocols.
 * 
 * @copyright Copyright (c) by Matthias Hertel, https://www.mathertel.de.
 *
 * This work is licensed under a BSD 3-Clause style license,
 * https://www.mathertel.de/License.aspx.
 * 
 * @brief
 *
 * This sample is part of the RFCodes library.
 * It demonstrates testing the code recognition in the SignalParser class
 * by injecting timings and verifying that codes could be detected.
 *
 * Use the Serial Monitor to see the received codes and send the predefined codes.
 *
 * More info at: http://www.mathertel.de/Arduino/
 * 
 * * 31.03.2032 created by Matthias Hertel
*/

#include <Arduino.h>
#include <SignalCollector.h>
#include <SignalParser.h>

#include <protocols.h>

bool showRaw = false;

SignalParser sig;
SignalCollector col;

// this is the test data with some annotations:
SignalParser::CodeTime testdata[] = {
    /* noise */ 13462, 70, 1433, 171, 232, 98, 1239, 337, 318, 52, 469, 182, 340, 432, 2860, 269, 4056, 108, 3290, 79, 2904, 260, 2870, 158, 7818, 75, 2047, 183, 520, 152, 161, 115, 114, 329, 340, 95, 4309, 153, 5210, 28, 2966, 273, 4856, 75, 955, 289, 333, 254, 433, 65, 129, 261, 609,

    520, 328, 1380, 1209, 513, 346, 1386, 1168, 534, 346, 1365, 1213, 507, 339, 1375, 346, 1372, 359, 13353, 392, 1317, 262, 48, 128, 95, 722, 466, 375, 1337, 1242, 492, 375, 1334, 388, 1350, 363, 1331, 1244, 478, 383, 1329, 389, 1329, 388, 1325, 1249, 477, 382, 1364, 1213, 522, 338, 1338, 1237, 496, 361, 1341, 1233, 493, 369, 1331, 1252, 465, 388, 1331, 1244, 469, 392, 1327, 390, 1322,

    // submitting it1 3 times:
    397, 13320,
    427, 1288, 1280, 446, 410, 1306, 1269, 451, 403, 1313, 409, 1308, 407, 1308, 1270, 452, 406, 1306, 412, 1309, 409, 1303, 1273, 446, 411, 1308, 1278, 442, 405, 1308, 1263, 456, 409, 1308, 1272, 448, 409, 1306, 1268, 464, 397, 1309, 1271, 448, 404, 1311, 407, 1310,
    // find: [it1 B001010000001]
    397, 13320,
    // find: [sc5 ff0f0ffffff0S]
    427, 1288, 1280, 446, 410, 1306, 1269, 451, 403, 1313, 409, 1308, 407, 1308, 1270, 452, 406, 1306, 412, 1309, 409, 1303, 1273, 446, 411, 1308, 1278, 442, 405, 1308, 1263, 456, 409, 1308, 1272, 448, 409, 1306, 1268, 464, 397, 1309, 1271, 448, 404, 1311, 407, 1310,
    // find: [it1 B001010000001]
    408, 13334,
    // find: [sc5 ff0f0ffffff0S]
    427, 1288, 1280, 446, 410, 1306, 1269, 451, 403, 1313, 409, 1308, 407, 1308, 1270, 452, 406, 1306, 412, 1309, 409, 1303, 1273, 446, 411, 1308, 1278, 442, 405, 1308, 1263, 456, 409, 1308, 1272, 448, 409, 1306, 1268, 464, 397, 1309, 1271, 448, 404, 1311, 407, 1310,
    // find: [it1 B001010000001]

    /* noise */ 445, 80, 1296, 128,

    443, 13281,
    460, 1257, 1313, 403, 464, 1254, 1310, 412, 450, 1263, 1311, 410, 449, 1265, 449, 1269, 452, 1265, 454, 1265, 446, 1267, 1312, 408, 452, 1264, 1310, 409, 451, 1265, 1311, 410, 448, 1267, 1305, 417, 443, 1271, 1306, 411, 445, 1274, 1308, 410, 443, 1276, 442, 1271,
    // find: [it1 B000110000001]

    /* noise */ 70, 232,

    // ideal, valid sc5: [sc5 0001100000011S]
    350, 1080, 350, 1080, 350, 1080, 350, 1080, 350, 1080, 350, 1080, 1080, 350, 1080, 350,
    350, 1080, 350, 1080, 350, 1080, 350, 1080, 350, 1080, 350, 1080, 350, 1080, 350, 1080,
    350, 1080, 350, 1080, 350, 1080, 350, 1080, 350, 1080, 350, 1080, 1080, 350, 1080, 350,
    350, 10912,
    // find: [

    /* noise */ 70, 1433, 171, 232, 98, 1239, 337, 318, 52, 955, 289, 333, 254, 433, 65,

    327, 2760, 326, 246, 324, 1316, 332, 1312, 320, 249, 332, 1307, 323, 257, 328, 237, 329, 1318, 327, 239, 323, 1324, 323, 1309, 336, 246, 327, 1304, 336, 256, 313, 243, 327, 1331, 324, 246, 323, 1322, 322, 1314, 324, 254, 326, 242, 324, 1317, 330, 245, 323, 1323,
    322, 1308, 325, 252, 323, 1315, 324, 258, 316, 1316, 328, 253, 320, 1313, 325, 262, 326, 244, 329, 1316, 325, 242, 330, 1322, 322, 240, 325, 1320, 326, 244, 325, 1328, 316, 1311, 330, 247, 326, 1310, 324, 257, 319, 251, 321, 1326, 318, 254, 315, 1340, 319, 1312,
    319, 265, 320, 243, 320, 1321, 325, 246, 323, 1321, 325, 247, 317, 1329, 314, 254, 321, 1320, 324, 249, 316, 1329, 318, 249, 319, 1328, 320, 249, 314, 1318, 321, 8387,
    // find: [it2 s_##__##__#__####____##__#_______x]

    /* noise */ 70, 1433, 171, 232, 98, 1239, 337, 318, 52, 955, 289, 333, 254, 433, 65,

    305,
    327, 2760, 326, 246, 324, 1316, 332, 1312, 320, 249, 332, 1307, 323, 257, 328, 237, 329, 1318, 327, 239, 323, 1324, 323, 1309, 336, 246, 327, 1304, 336, 256, 313, 243, 327, 1331, 324, 246, 323, 1322, 322, 1314, 324, 254, 326, 242, 324, 1317, 330, 245, 323, 1323,
    322, 1308, 325, 252, 323, 1315, 324, 258, 316, 1316, 328, 253, 320, 1313, 325, 262, 326, 244, 329, 1316, 325, 242, 330, 1322, 322, 240, 325, 1320, 326, 244, 325, 1328, 316, 1311, 330, 247, 326, 1310, 324, 257, 319, 251, 321, 1326, 318, 254, 315, 1340, 319, 1312,
    319, 265, 320, 243, 320, 1321, 325, 246, 323, 1321, 325, 247, 317, 1329, 314, 254, 321, 1320, 324, 249, 316, 1329, 318, 249, 319, 1328, 314, 1318, 320, 249, 321, 8387,
    // find: [it2 s_##__##__#__####____##__#______#x]

    /* noise */ 589, 396, 595, 377, 1077, 878, 1086, 375, 55568,

    1044, 918, 1021, 940, 1003,
    462, 509, 470, 492, 980, 486, 487, 972, 987, 477, 506, 456, 525, 945, 1007, 456, 533, 436, 533, 440, 535, 448, 531, 925, 1023, 935, 539, 438, 531, 440, 1028, 931, 1014, 447, 535, 930, 1028, 924, 1027, 437, 548, 441, 526, 924, 1038, 431, 536, 439, 535, 447, 538,
    926, 1019, 941, 1016, 439, 537, 445, 533, 927, 530, 446, 536, 438, 536, 443, 533, 451, 535, 440, 1015, 934, 1020, 930, 537, 443, 1028, 436, 530, 450, 532, 442, 535, 437, 537, 450, 530, 444, 535,
    // find: [cw Hsslsllssllsssslllsslllsllllssllsssllllsslsssssllllslssssss]

    /* noise */ 941, 1016, 439, 537,

    0};

const char *testresult[] = {
    "it1 B001010000001",
    "sc5 ff0f0ffffff0S",
    "it1 B001010000001",
    "sc5 ff0f0ffffff0S",
    "it1 B001010000001",
    "it1 B000110000001",
    "sc5 000100000001S",
    "it2 s_##__##__#__####____##__#_______x",
    "it2 s_##__##__#__####____##__#______#x",
    "cw Hsslsllssllsssslllsslllsllllssllsssllllsslsssssllllslssssss",
    ""};

int nextResult;



// This function will be called when a complete protcol was received.
void receiveCode(const char *proto)
{
  const char *expect = testresult[nextResult];
  if ((*expect) && strcmp(expect, proto) == 0) {
    Serial.printf("\n ok : [%s]\n", proto);
    nextResult++;
  } else {
    Serial.printf("\n BAD: [%s]\n", proto);
    Serial.printf(" exp: [%s]\n", expect);
  }
} // receiveCode()


void setup()
{
  delay(3000); // wait for serial monitor to be ready.
  Serial.begin(115200);
  Serial.println("RFCodes code recognition test...");
  Serial.println();

  Serial.println(
      "Commands: T(est data)");

  // initialize the SignalCollecor library without hardware
  col.init(&sig, NO_PIN, NO_PIN); // no pins. for testing purpose.

  // load all codes that may be detected in the test data
  sig.load(&it1);
  sig.load(&it2);
  sig.load(&sc5);
  sig.load(&cw);

  // show loaded protocols
  sig.dumpTable();

  sig.attachCallback(receiveCode);
} // setup()


void loop()
{
  unsigned long now = millis();
  static unsigned long nextReport = now + 1000;

  if (Serial.available() > 0) {
    char cmd = Serial.read();
    if (cmd == '1') {

    } else if (cmd == 'T') {
      Serial.println("Sending Test Data...");
      nextResult = 0;
      SignalParser::CodeTime *d = testdata;
      while (*d) {
        Serial.print(*d);
        Serial.print(',');
        col.injectTiming(*d++); // add timing
        col.loop();             // let it parse
      }                           // while

    } // if
  }   // if

  // process received bytes
  col.loop();
} // loop()
