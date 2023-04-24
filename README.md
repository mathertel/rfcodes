# RFCodes library

This is a library that can encode and decode signals patterns that are used in the 433 MHz and IR technology.

These signals use a carrier frequency (433MHz or 44kHz) that is switched on and off using a defined pattern.

Each protocol often defined by a device manufacturer or a chip producing company consists of a series of pulses and pauses (codes) with a defined length that have a special semantic.

There is a textual representation for sending and receiving a sequence by specifying the short name of the protocol and the characters specifying the code. For some protocols there is an algorithm defined that compiles the characters into a real value.
So any code sequence corresponds to a textual representation like `it2 s_##___#____#_#__###_____#__#____x`

## Use the library

Using the library requires the following steps:

* A set of protocol definitions that you expect that they are used.
* A pin where the inbound signal comes in (receiver)
* A pin where the outbound signal can be send (sender)
* A function that gets called when a code was decoded.

For the receiver role the library uses a interrupt routine that gets called when ever a signal change on the pin has been detected.
Some microprocessors support only specific pins with interrupts
so please look up the documentation for **Arduino attachInterrupt()** function for the processor.

Sending a protocol uses no interrupts but also should not be interrupted by another ISR routine.

## The Wiring

A interrupt capable pin can be used to attach a receiver. e.g. D5 on a ESP8266 board.

Another pin can be used to attach a transmitter e.g. D6 on a ESP8266 board.

Both, receiver and transmitter must be connected to VCC and GND.
Best option is to use variants that can be used with 3.3V when using a ESP8266.

```TXT
     3.3V ---------------- 3.3V --------------- 3.3V
      |                     |                    |  
+-----+-----+         +-----+-----+         +----+------+
| RF433     |         | ESP8266   |         | RF433     |
| receiver  +----> D5-+ board     +-D6 ---->+ sender    |  
| module    |         |           |         | module    |
+-----+-----+         +-----+-----+         +----+------+
      |                     |                    |  
     GND ----------------- GND ---------------- GND
```

The receiver modules that can be used must detect the RF signal and produce a signal when the carrier frequency has been detected. The polarity of the signal is not relevant.
The modules I used and found reliable are the type RXB8 and RXB12, both with a ceramic resonator. The RF-5V and XY-MK-5V modules were not reliable in my environment and setup.

The sender modules must produce a carrier frequency on HIGH output. When not transmitting a code the output is LOW so other devices can use the carrier frequency on their own. I used several modules all with ceramic resonators (no adjustable air coils). They seem to be less critical.

## Examples

The following examples sketches are available:

* The [Intertechno](./examples/intertechno/README.md) example shows how to register and use
    the 2 protocols used by devices from intertechno.

* The [TempSensor](./examples/TempSensor/README.md) example shows how to receive temperature+humidity
  from a cresta protocol based sensor.

* The [necIR](./examples/necIR/README.md) example shows how to receive and send the Infrared NEC protocol.

* The [Scanner](./examples/scanner/README.md) example can be used to collect code timings for further analysis.

## Protocol definitions

Here are some hints on how to configure a protocol:

In the protocol structure the (short) name of the protocol and some
settings must be defined:

* **name** - a short name of the protocol.
* **minCodeLen** - the minimum length of a code sequence including all start, data and end codes.
* **maxCodeLen** - the maximum length of a code sequence including all start, data and end codes.
* **tolerance** - codes are not sent and not captured using very precise timings. The tolerance defines the percentage the timing may derive.
* **sendRepeat** - When sending the code the sequence should be repeated as specified by the sendRepeat parameter.
* **baseTime** - Many protocols use a base clock time. This should be specified in the baseTime parameter and the factors in the code.
* **codes** -  The list of codes in this protocol.

In the code definitions the typical timing patterns are defined.

* **type** - The code type defines the role of this code in the sequence.

  * A **start code** is defined to recognize that a protocol is send out as it is sent at the beginning only.
    This can be used to simply wait until such a unique timing can be found. This may be a code with a exceptional duration or a series of durations marking the start of a sequence.
    As the following codes often have shorter timings so a
    short pulse and a long pause is part of many protocols to detect other senders transmitting into the pause. A simple way of collision detection.

  * Multiple **data codes** are defined to represent data bits.
    For binary protocols one sequence stays for a set bit and another for a cleared bit but you can find also protocols with 3 data codes. Multiple of these codes in a row can then be used to build the protocol data.

  * A **end code** marks the end of a sequence.
    This is useful for protocols that have a variable length of data.
    A code defined with the END flag will always stop the current sequence detection. When the minimum length is not yet given the sequence is not taken as a valid code.

  * Codes with a **fixed length** are defined using the minimum and maximum length with the length of the sequence. Do use the END flag only when there is a special code defined marking the end.
    Some protocols just end after a number of codes.

* **name** - The single character representing a code in the sequence.
It must be unique within the protocol.

* **durations** - The list of durations that represent the code.

### Protocol Example

The code used by the SC5272 chip is named "sc5", has 3 data codes ('0', '1' and 'f') and a stop code ('S') defined.
So the textual representation may be "[sc5 0000f0000fffS]"

The chip can be used with different clock speeds so the baseTime can be adjusted to fit the speed of your device.

```CPP
/** Definition of the protocol from SC5272 and similar chips with 32 - 46 data bits data */
SignalParser::Protocol sc5 = {
    "sc5",
    .minCodeLen = 1 + 12,
    .maxCodeLen = 1 + 12,

    .tolerance = 25,
    .sendRepeat = 3,
    .baseTime = 100,
    .codes = {
        {SignalParser::CodeType::ANYDATA, '0', {4, 12, 4, 12}},
        {SignalParser::CodeType::ANYDATA, '1', {12, 4, 12, 4}},
        {SignalParser::CodeType::ANYDATA, 'f', {4, 12, 12, 4}},
        {SignalParser::CodeType::END, 'S', {4, 124}}}};
```

This 3-state protocol is also found using the END code as a start code. When submitting multiple sequences in a row as it is usually done by senders and expected by receivers this protocol is partially equivalent to the `it1` protocol.

## Implementation

There are 2 classes combined here:

**SignalParser**

The `SignalParser` is a general usable class that knows all about the timing of codes in the protocol
and knows how to decode and encode them.

This class can take pulse/gap durations give to the `parse()` method and uses the registered protocol definitions
to detect a full protocol sequence using valid codes.
This allows a flexible usage of the SignalParser to be combined with different signal sources and frequencies
or use the class to test for codes in a given series of durations. (see Example testcodes.ino)

Since the solutions of the manufacturers vary quiet a lot this library can be adapted to different protocols by registering the signal patterns of the protocols using the `load` method by passing a Protocol+Codes definition.

Whenever a full sequence is detected from the given durations the callback function is used to pass the sequence over for further processing.

```CPP
SignalParser sig;

// load some protocols into the SignalParser
sig.load(&RFCodes::it1);
sig.load(&RFCodes::it2);

// register the callback function.
sig.attachCallback(receiveCode);
```

**SignalCollector**

The `SignalCollector` class handles interrupt routines and the IO pins.
Every time when receiving a signal change the duration since the previous change is collected into a buffer.

The loop() function must be called from the main loop function to transfer the durations from the buffer into the parser.

Sending a sequence is done by calling the send() function with the protocol name and the codes as a string.

```CPP
SignalCollector col;

// initialize the SignalCollector library
col.init(&sig, D5, D6); // input at pin D5, output at pin D6

// send a sequence
col.send("it2 s_##___#____#_#__###_____#____#__x");
```

## See also

* [About RF Protocols](/docs/rf433.md)
* [Standard protocols](/docs/SC5272_protocol.md)
* [EV1527 protocol](/docs/ev1527_protocol.md)
* [intertechno protocols](/docs/intertechno_protocol.md)
* [Cresta protocol for sensors](/docs/cresta_protocol.md)
