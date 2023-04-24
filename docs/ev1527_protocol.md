# EV1527 Protocol

The protocol that the EV1527 chip is one of the kind that has a (almost) unique ID as part of the sending sequence
to avoid accepts conflicts with other implementations.

The receivers must "learn" these IDs usually by using a specific button on the devices.

From the datasheet you can see that the a start code and 24 data codes are sent.

## Protocol Timings

From the manual you can find 3 different codes in use:

| Code | timing ratio    |
| -----| --------------- |
| 's'  | 1 : 32 |
| '0'  | 1 :  3 |
| '1'  | 3 :  1 |

A fully frame will consist in a

* 's' preample
* 10 digits that make a unique sender ID
* 4 digits that vary by the input signal (button) used.

These ratios need to be multiplied by a base timing that varies.

The EV1527 protocol can be added to the signal parser by calling `sig.load(&RFCodes::ev1527);`

The baseTiming of 320 µ seconds is a good starting point but may be adjusted in the protocol defintion.

## Examples

    [ev1527 s100100110110001001000001]
    [ev1527 s100100110110001001000010]

## See also

* EV1527 datasheet <https://www.sunrom.com/download/EV1527.pdf>
