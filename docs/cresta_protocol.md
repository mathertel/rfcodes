# Cresta protocol for sensors

This protocol definition supports the manchester encoded protocol that is used in some temperature and humidity sensors using 433 MHz transmission.

These sensors have a fixed starting byte that is transferred using 5 long timings in a row.
The following timings are either one long (l) or two short timings (s) as defined in the manchester protocol.
See the wikipedia article <https://en.wikipedia.org/wiki/Manchester_code> on how the manchester code is built in principle.

From the cresta data in the manchester coding a 7 bytes are recorded even when the sensor will send more information and a checksum.
More details on the protocol can be found in <http://members.upc.nl/m.beukelaar/Crestaprotocol.pdf>.


## How manchester code can be decoded based on fixed codes

The recorded sequence begins with the data bit 0x75 transferred with the LSB first and a 0-bit between the bytes: `0 1-0-1-0-1-1-1-0 0`. 
This results in Huffman coding: `LH HL-LH-HL-LH-HL-HL-HL-LH LH`

When two signals in a row with the same level will result in a long and others in short timings: 
* A `H-H` or `L-L` sequence is recorded as one long signals (l)
* A `H-L` or `L-H` sequence is recorded as two short singals (s)

The timings we record is `(s)l  l  l  l  l  s  s  l  s..`.
This is well designed so we can wait until we see the 5 long signals in a row and than start capturing the l or s codes.
Therefore signals from the cresta type sensors will start with `[cw Hssls` 

```TXT
data  :  0  -------- 0x75 --------  0
data  :  0  1  0  1  0  1  1  1  0  0
level :  LH HL LH HL LH HL HL HL LH LH
times :  sl -l -l -l -l -s ss sl -s ss 
code  :  -H--------------s--s--l--s--s 
```

The package starts with a first short low level `s` that typically follows a pause so it will not be observed.
The next 5 `l` in a row are combined into the start code `H` as a the start sequence when receivong a data package.

The `cresta_decode` function takse the codes as an input and creates the data bytes by decoding into levels and bits.
Then the temperature and humidity is takenfrom the data bits and printed to the log.


### Protocol Example with decoding

    [cw Hsslsllssllsssslllsslllsllllsllsllsslssslslsssslllslsslssss]
    data:9f 33 ce de 13 c2 12 
    .temp: 21.3 °C
    .hum: 12 %

## See also

* <http://members.upc.nl/m.beukelaar/Crestaprotocol.pdf>
 
