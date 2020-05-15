# Cresta protocol 

This protocol definition supports the manchester encoded protocol that is used in some temperature and humidity sensors using 433 MHz transmission.

These sensors have a fixed starting byte that is transferred using 5 long timings in a row.
The following timings are either one long (l) or two short timings (s) as defined in the manchester protocol.
See the wikipedia article <https://en.wikipedia.org/wiki/Manchester_code> on how the manchester code is built in principle.

From the cresta data in the manchester coding a 7 bytes are recorded even when the sensor will send more information and a checksum.
More details on the protocol overall can be seen in <http://members.upc.nl/m.beukelaar/Crestaprotocol.pdf>.

## How manchester code can be decoded based on fixed codes

The recorded sequence begins with the data bit 0x75 transferred with the LSB first and a 0-bit inbetween the byte: `0 1-0-1-0-1-1-1-0 0`. 
This results in Huffman coding: `LH HL-LH-HL-LH-HL-HL-HL-LH LH`

When two signals in a row with the same lavel will result in a long and others in short timings: 
* A `H-H` or `L-L` sequence is recorded as one long signals (l)
* A `H-L` or `L-H` sequence is recorded as two short singals (s)

The timings we record is `(s)l  l  l  l  l  s  s  l  s..`.
This is well designed so we can wait until we see the 5 long signals in a row and than start captuiring the l or s codes.
Therefore signals from the cresta type sensors will start with `[cw Hssls` 


## Protocol Example with decoding

    [cw Hsslsllssllsssslllsslllsllllsllsllsslssslslsssslllslsslssss]
    data:9f 33 ce de 13 c2 12 
    .temp: 21.3 °C
    .hum: 12 %

To decode the temperature and humidity see the `cresta_decode()` function in the example `TempSensor.ino`.


## See also

* <http://members.upc.nl/m.beukelaar/Crestaprotocol.pdf>
 










[cw Hsslsllssllsssslllsslllsllllsllsllsslssslslsssslllslsslssss]
data:9f 33 ce de 13 c2 12 
.temp: 21.3 °C
.hum: 12 %

check 9f, 13

BAD:

[cw Hlsslsllssllsssslllsslllsllllsllsllssllsssslssssllllslsssss]
data:3f 67 9d bd 67 85 0b 
.temp: 56.7 °C
.hum: 11 %


