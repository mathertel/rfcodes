# Intertechno 2 Protocol

This protocol is described at some places in projects and is obviously used in different brands.
It uses a Manchester encoding scheme with some special behavior regarding the dimming of lights.
The receiving units like switches and dimmers do have a learning mode and can accept commands using multiple  ids because the senders do have a fixed and unique id.

## ON/OFF Protocol

The ON/OFF protocol is using a sequence of 34 codes:

* Starting code of type 's'.
* 26 codes of type '#' or '_' that represent the 26 bit unique id of the sender.
* 1 code of type '#' or '_' used as a flag to address all receivers programmed to any button.
* 1 code to switch the device on ('#') or off ('_')
* 4 codes of type '#' or '_' specifying what button was pressed.
* Terminating code of type 'x'.

bit 0..25  : unique ID of the transmitter device
// bit 26     : group flag, if set, all reivers with the same id must act
// bit 27     : 0= OFF, 1=ON, D=dimming
// bit 28..31 : unit number
// bit 32..35 : dim level

## Dimming Protocol

The dimming protocoll is using a sequence of 38 codes:

* Starting code of type 's'.
* 26 codes of type '#' or '_' that represent the 26 bit unique id of the sender.
* 1 code of type '#' or '_' used as a flag to address all receivers programmed to any button.
* 1 code of type 'D' specifying that this is a dimming command.
* 4 codes of type '#' or '_' specifying what button was pressed.
* 4 codes of type '#' or '_' specifying the dimming level.
* Terminating code of type 'x'.

## Codes

There are some

``` cpp
#define B1_on  "s_##___#____#_#__###_____#__#____x"
#define B1_off "s_##___#____#_#__###_____#_______x"

#define B4_on  "s_##___#____#_#__###_____#__#__##x"
#define B4_off "s_##___#____#_#__###_____#_____##x"
#define B4_d   "s_##___#____#_#__###_____#__D__##_#__x"

//

#include "TabRF.h"

#define IT2_LEN 275

Code Intertechno2_Codes[] = {
// starting sequence is /‾\__________/
{TabRF_CODE_START, 's', 2, {IT2_LEN, 10 * IT2_LEN}},

    // low signal is /‾\_/‾\_____/
    {TabRF_CODE_ANY, '_', 4, {IT2_LEN, IT2_LEN, IT2_LEN, 5 * IT2_LEN}},

    // high signal is /‾\_____/‾\_/
    {TabRF_CODE_ANY, '#', 4, {IT2_LEN, 5 * IT2_LEN, IT2_LEN, IT2_LEN}},

    // DIM signal is /‾\_/‾\_/  (shorter !)
    {TabRF_CODE_ANY, 'D', 4, {IT2_LEN, IT2_LEN, IT2_LEN, IT2_LEN}},

    // end signal is /‾\_/‾\______40 times____/  (longer !)
    {TabRF_CODE_END, 'x', 2, {IT2_LEN, 10728}}

};

Sequence Intertechno2_Sequence = {
8, // min. Code length
38, // max code length
4, // tolerance
3, // send repeat

sizeof(Intertechno2_Codes) / sizeof(Code), // # of code definitions following...

Intertechno2_Codes
}; // Sequence
```

## References and See Also

* <http://playground.arduino.cc/Code/HomeEasy>
* <http://elektronikforumet.com/wiki/index.php/RF_Protokoll_-_Nexa/Proove_(äldre,_ej_självlärande)>
* <https://github.com/calle-gunnarsson/NexaCtrl>
* <http://homeeasyhacking.wikia.com/wiki/Advanced_Protocol>
* <https://github.com/pilight/pilight>
* <https://manual.pilight.org/protocols/433.92/switch/intertechno.html>
* <https://wiki.fhem.de/wiki/Intertechno_Code_Berechnung>
* <https://bitbucket.org/fuzzillogic/433mhzforarduino/wiki/Home>

* <https://www.sweetpi.de/blog/329/ein-ueberblick-ueber-433mhz-funksteckdosen-und-deren-protokolle>

Scanner from Suat Özgür:
<https://github.com/sui77/SimpleRcScanner>
<http://test.sui.li/oszi/Sketchbook/SimpleRcScanner.ino>
<http://test.sui.li/oszi/>
