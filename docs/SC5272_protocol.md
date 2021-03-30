# SC5272 Protocol

The protocol that the SC5272 chip is one of the kind that accepts some adress and data inputs and generates a 12-bit protocol
using different ON/OFF signal patterns.

This chip allows 3 different states per bit.

From the datasheet you can see that the sync code is sent after the data bits and the whole protocol is sent 3 times in a row.
Therefore othr protocol definitions with a starting sync code may conflict. E.g. see intertechno-old protocol. 

There are also very similar chips like the PT2264 on the market that also send these kind of signals and you can find also derivations of this protocol.

## Protocol Timings

From the manual you can find 4 different codes in use:

| Code     | timing ratio    |
| -------- | --------------- |
| bit '0'  | 4 : 12 : 4 : 12 |
| bit '1'  | 12 : 4 : 12 : 4 |
| bit 'f'  | 4 : 12 : 12 : 4 |
| sync bit | 4: 124          |

These rations need to be multiplyed by a base timing that varies.

The SC5272 protocol can be added to the signal parser by calling `register_SC5272(&sig, baseTiming = 100)`

The baseTiming of 100 µ seconds is a good starting point but may be adjusted using the optioal poarameter.

## Examples

    [sc5 0000f0000fffS]
    [sc5 0000f0000ff0S]


## See also

* SC5272 datasheet <https://old.domotronic.fr/telechargement/SC5272S-L4datasheet.pdf>
* PT2264 datasheet <http://www.princeton.com.tw/Portals/0/Product/PT2264_1.pdf>
