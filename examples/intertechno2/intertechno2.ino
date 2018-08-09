/*
  file: intertechno2.ino
  This sample is part of the TabRF library showing how to receive and send
  intertechno2 compatible codes. The library internals are explained at
  http://www.mathertel.de/Arduino/TabRFlibrary.aspx

  This is a sketch that listens to RF Signals and sends decoded protocols to the
  Serial output.

  Setup a test circuit:

  Connect the data pin of the receiver with
  * Arduino UNO pin #2
  * ESP8266 pin D1
  Connect the data pin of the sender with
  * Arduino UNO pin #9
  * ESP8266 pin D4
  or adjust the pins in the parameters when calling the tabRF.init() function.

  Use the Serial Monitor to see the received codes and send the predefined
  codes.

  25.03.2018 ported to ESP8266
  29.03.2018 converted to library
*/

#include <TabRF.h>
#include <intertechno2.h>

char lastCode[MAXCODES]; // last received protocol
boolean lastCodeReceived = false;

// decoded parts of the protocol in numbers
unsigned long lastID = 0;
unsigned int lastUnit = 0;
unsigned int lastDim = 0;

// This function will be called when a complete protcol was received.
void receiveCode(char *proto)
{
  // remember last code in a local variable
  strncpy(lastCode, proto, sizeof(lastCode));
  lastCodeReceived = true;
} // receiveCode()


void setup()
{
  Serial.begin(115200);
  Serial.println();
  Serial.println("TabRF Packet Receiver");

  Serial.println(
      "Commands: 1-9(Send Code) T(Dump Code Table) L(Decode last code)");

  // initialize the tabRF library
  tabRF.init(D1, D4); // input at pin #2 , output a pin # 9
  tabRF.setupDefition(&Intertechno2_Sequence);
  tabRF.attachCallback(receiveCode);
} // setup()

#define B1_on  "s_##___#____#_#__###_____#__#____x"
#define B1_off "s_##___#____#_#__###_____#_______x"

#define B4_on  "s_##___#____#_#__###_____#__#__##x"
#define B4_off "s_##___#____#_#__###_____#_____##x"
#define B4_d   "s_##___#____#_#__###_____#__D__##_#__x"

void loop()
{
  if (Serial.available() > 0) {
    char cmd = Serial.read();
    if (cmd == '1') {
      Serial.println("Sending (1)...");
      tabRF.send(B4_on);

    } else if (cmd == '2') {
      Serial.println("Sending (2)...");
      tabRF.send(B4_off);

    } else if (cmd == '5') {
      Serial.println("Sending (5)...");
      tabRF.send("s_##___#____#_#__###_____#__D__##_#__x");

    } else if (cmd == 'T') {
      tabRF.dumpTable();

    } else if (cmd == 'L') {
      Serial.println("Last Code decoded:");
      Serial.println(lastCode);

      if (lastCode[28] == 'D') {
        Serial.println("s<---   id             --->GD<un><di>x");
      } else {
        Serial.println("s<---   id             --->GL<un>x");
      }

      // bit 0      : start
      // bit 1..26  : unique ID of the transmitter device
      // bit 27     : group flag, if set, all reivers with the same id must act
      // bit 28     : 0= OFF, 1=ON, D=dimming
      // bit 29..32 : unit number
      // bit 33..36 : dim level
      // x

      // decode the id to a number:
      lastID = 0;
      for (int n = 1; n <= 26; n++) {
        lastID = lastID << 1;
        if (lastCode[n] == '#')
          lastID += 1;
      } // for
      Serial.print("id:    ");
      Serial.println(lastID, DEC);

      // Decode the group flag.
      if (lastCode[27] == '#') {
        Serial.println("group: Group flag is on all devices must act.");
      } else {
        Serial.println("group: No Group flag set.");
      }

      // Decode the Level flag.
      if (lastCode[28] == '#') {
        Serial.println("level: Switch Device on.");
      } else if (lastCode[28] == '_') {
        Serial.println("level: Switch Device off.");
      } else if (lastCode[28] == 'D') {
        Serial.println("level: Dim the Device.");
      } // if

      // Decode the unit number.
      lastUnit = 0;
      for (int n = 29; n <= 32; n++) {
        lastUnit = lastUnit << 1;
        if (lastCode[n] == '#')
          lastUnit += 1;
      }
      Serial.print("unit:  ");
      Serial.println(lastUnit, DEC);

      lastDim = 0;
      Serial.print("dim:   ");
      Serial.println(lastDim, DEC);

      //       <---   id             --->GS<un><di>
      // 34: [s_#_###__#_#__#_##_#_###_#_x_#__#_x] on

    } // if
  } // if

  if (lastCodeReceived) {
    // a new code was received
    Serial.print('[');
    Serial.write(lastCode);
    Serial.println(']');
    lastCodeReceived = false;
  } // if

  delay(5);
} // loop()
