# Intertechno Protocols

The RF devices from Intertechno use two different protocols.

Both protocols are described at some places in projects and are obviously also used by other brands.


## Intertechno 1

The older and simpler protocol uses a fixed length of 1+12 codes done with a 2 pulse start code and 4 pulse data codes.

Setting the codes is done by specifying the unit through the first 5 codes and the ID in the following 5 codes.
2 codes can be used for the data and is often used for ON/OFF protocols using the last code. 

As you can see in the example there is a high risk that another installation in the neighborhood can interfere.

### Examples

    [it1 B000110000000] unit:1, key:1 on
    [it1 B100010000001] unit:1, key:1 off
    [it1 B010010000001] unit:2, key:1 off
    [it1 B001010000001] unit:3, key:1 off
    [it1 B000110000001] unit:4, key:1 off


## Intertechno 2

The newer and longer code supporting 32 and 36 data bits
gives much more space for units but also takes longer for transmitting a single sequence.

The devices have a build in fixed id using 26 codes. It uses a Manchester encoding scheme
with start and stop sequences and a special behavior regarding the dimming of lights.

The receiving units like switches and dimmers do have a learning mode and can accept commands from multiple sender IDs.

### Examples

    // ITT-1500, no switch, 3 key pais and "all off"
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

### Dimming Protocol Details

The dimming protocoll is using a sequence of 38 codes:

* Starting code of type 's'.
* 26 codes of type '#' or '_' that represent the 26 bit unique id of the sender.
* 1 code of type '#' or '_' used as a flag to address all receivers programmed to any button.
* 1 code of type 'D' specifying that this is a dimming command.
* 4 codes of type '#' or '_' specifying what button was pressed.
* 4 codes of type '#' or '_' specifying the dimming level.
* Terminating code of type 'x'.

## Protocol activation

To activate the intertechno RF protocols you have to include `<intertchno.h>`
and call `register_intertechno1()` and / or `register_intertechno2()` to activate the 2 protocols.


## See also

* <https://www.intertechno24.de/>

Descriptions and projects found on the internet:

* <https://manual.pilight.org/protocols/433.92/switch/intertechno_old.html> (it1)
* <https://github.com/pimatic/rfcontroljs/blob/master/src/protocols/switch4.coffee> (it1)

* <https://manual.pilight.org/protocols/433.92/switch/intertechno.html> (it2)
* <https://github.com/pimatic/rfcontroljs/blob/master/src/protocols/switch1.coffee> (it2)

* <https://wiki.fhem.de/wiki/Intertechno_Code_Berechnung>

* <https://bitbucket.org/fuzzillogic/433mhzforarduino/wiki/Home>
* <https://homeeasyhacking.fandom.com/wiki/Advanced_Protocol>


## Examples

```TXT
intertechno ITT-1500 sender (außen)
              s<---   id             --->GS<un>x
ch1-on:  [it2 s_##__##__#__####____##__#__#____x] // signal:on, unit:0
ch1-off: [it2 s_##__##__#__####____##__#_______x] // signal:off, unit:0
ch2-on:  [it2 s_##__##__#__####____##__#__#___#x] // signal:on, unit:1
ch2-off: [it2 s_##__##__#__####____##__#______#x]
ch3-on:  [it2 s_##__##__#__####____##__#__#__#_x] // signal:on, unit:2
ch3-off: [it2 s_##__##__#__####____##__#_____#_x]
all-off: [it2 s_##__##__#__####____##__#_#_____x] // global off


intertechno ITLS-16 sender, switch: II (adds 4 to unit)
              s<---   id             --->GS<un>x
ch1-on:  [it2 s_##___#____#_#__###_____#__#_#__x]] // signal:on, unit:4+0
ch1-off: [it2 s_##___#____#_#__###_____#____#__x]] // signal:off
ch2-on:  [it2 s_##___#____#_#__###_____#__#_#_#x]] // unit:4+1
ch2-off: [it2 s_##___#____#_#__###_____#____#_#x]]
ch3-on:  [it2 s_##___#____#_#__###_____#__#_##_x]] // unit:4+2
ch3-off: [it2 s_##___#____#_#__###_____#____##_x]]
ch4-on:  [it2 s_##___#____#_#__###_____#__#_###x]] // unit:4+3
ch4-off: [it2 s_##___#____#_#__###_____#____###x]]

intertechno ITM-100, window / door sensor, switch: D3 
[it1 B001110111001] close
[it1 B001110111000] open

intertechno ITM-100, window / door sensor, switch: A2
[it1 B0000f0000ff0] close
[it1 B0000f0000fff] open
```
