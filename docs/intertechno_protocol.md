# Intertechno Protocols

The RF devices from Intertechno use two different protocols.

Both protocols are described at some places in projects and are obviously also used by other brands.


## Intertechno 1

The older and simpler protocol uses a fixed length of 1+12 codes done with a 2 pulse start code and 4 pulse data codes.

Setting the codes is done by specifying the unit through the first 5 codes and the ID in the following 5 codes.
2 codes can be used for the data and is often used for ON/OFF protocols using the last code. 

By the limited number of codes that can be configured on the devices
there is a high risk that another installation in the neighborhood can interfere.

### Examples

```TXT
[it1 B000110000000] unit: 4, key: 1 on
[it1 B100010000001] unit: 1, key: 1 off
[it1 B010010000001] unit: 2, key: 1 off
[it1 B001010000001] unit: 3, key: 1 off
[it1 B000110000001] unit: 4, key: 1 off
```

There are manual senders with Units1-4 and others with units 0-15 so the actual bits may need to be interpreted differently.
Just record the sequence of the key you press and see.


### Timings

My observation is that the durations of the codes vary a lot during a sequence. 

Example:

    [it1 B001010000001]

    /* B */  433, 13291,
    /* 0 */  446,  1267,  1304,   419,
    /* 0 */  439,  1276,  1299,   422,
    /* 1 */  434,  1280,   437,  1281,
    /* 0 */  436,  1281,  1293,   426,
    /* 1 */  430,  1289,   427,  1287,
    /* 0 */  429,  1291,  1283,   432,
    /* 0 */  426,  1291,  1283,   441,
    /* 0 */  416,  1298,  1277,   442,
    /* 0 */  417,  1301,  1273,   447,
    /* 0 */  413,  1301,  1271,   449,
    /* 0 */  410,  1308,  1266,   459,
    /* 0 */  399,  1316,   399,  1316,

* The short durations vary from 399 to 459   (429) +/- 7%
* The long  durations vary from 1266 to 1316 (1291) +/- 2 %
* The ratio is about 1:3 and the long impulse for the start code is 1:31.

These timing rations resemble very much the protocol used in encoders like SC5272
and when inspecting the timings it seam that often the first sequence in not properly started.

Manual senders repeat as long as the button is pressed. When sent per library 4 codes in a row seems to fit. 


## Intertechno 2

The newer and longer code supporting 32 and 36 data bits gives much more space for units but also takes longer for transmitting a single sequence.

The devices have a build in fixed id using 26 codes.

It uses a Manchester encoding scheme
with start and stop sequences and a special behavior regarding the dimming of lights.

The receiving units like switches and dimmers do have a learning mode and can accept commands from multiple sender IDs.

### Examples

    // ITT-1500, no switch, 3 button pairs and one button "all off"
    [it2 s_##__##__#__####____##__#__#____x] key 1 on
    [it2 s_##__##__#__####____##__#_______x] key 1 off
    [it2 s_##__##__#__####____##__#______#x] key 2 off
    [it2 s_##__##__#__####____##__#_#_____x] all off

    // ITLS-16 with switch for 4 units
    [it2 s_##___#____#_#__###_____#__#____x] unit:1, key 1 on
    [it2 s_##___#____#_#__###_____#_______x] unit:1, key 1 off
    [it2 s_##___#____#_#__###_____#__#_#__x] unit:2, key:1 on
    [it2 s_##___#____#_#__###_____#____#__x] unit:2, key:1 off


### ON/OFF Protocol Details

The ON/OFF protocol is using a sequence of 34 codes:

* Starting code of type 's'.
* 26 data codes that represent the unique 26 bit id of the sender.
* 1 data code used as a flag to address all receivers, any button. 
* 1 data code to switch the device on ('#') or off ('_')
* 4 data codes specifying the pressed button / unit.
* 1 Terminating code of type 'x'.


### Dimming Protocol Details

In case of the dimming devices a specific dimming value can be set directly using a 38 code sequence.

This code uses a 'D' code (shorter than the others) instead of on/off to indicate the
transmission of a dim command. Before the terminating code 4 additional data codes are sent to
specify the dim level:

* Starting code of type 's'.
* 26 data codes that represent the unique 26 bit id of the sender.
* 1 data code used as a flag to address all receivers, any button. 
* 1 data code 'D'
* 4 data codes specifying the pressed button / unit.
* 4 data codes specifying the dim level.
* 1 Terminating code of type 'x'.


## loading the protocols

To activate the intertechno RF protocols you have to load them into the signal parser:

    signalParser.load(&it1); // loading Intertechno 1
    signalParser.load(&it2); // loading Intertechno 2


## See also

* <https://www.intertechno24.de/>

Descriptions and projects found on the internet:

* <https://manual.pilight.org/protocols/433.92/switch/intertechno_old.html> (it1)
* <https://github.com/pimatic/rfcontroljs/blob/master/src/protocols/switch4.coffee> (it1)

* <https://manual.pilight.org/protocols/433.92/switch/intertechno.html> (it2)
* <https://github.com/pimatic/rfcontroljs/blob/master/src/protocols/switch1.coffee> (it2)

* <https://wiki.fhem.de/wiki/Intertechno_Code_Berechnung>

