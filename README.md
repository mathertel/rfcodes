# TabRF => RF

This is a library that can encode and decode signals patterns that are used in the 433 MHz and IR technology.

These signals use a carrier frequency (433MHz or 44kHz) that is switched on and off using a defined pattern.

Each protocol often defined by a device manufacturer or a chip producing company consists of a series of pulses and pauses (codes) with a defined length that have a special semantic:

* A **start code** is defined to recognize that a protocol is send out.
This can be used to simply wait until such a unique timing can be found.
The following codes often have shorter timings.
Typically a short pulse and a long pause is part of the start code so another sender on the same frequency will break detecting the start condition.

* Multipe **data codes** are defined to represent data bits. For binary protocols one sequence stays for a set bit and another for a cleared bit but you can find also protocols with 3 data codes. Multiple of these codes in a row can then be used to build the protocol data.

* A **stop code** is defined for protocols that have a variable length of data.
This is not strictly required when a fixed data length is defined so some protocols just end after a number of data sequences.

There is a textual representation for sending and receiving a sequence by specifying the short name of the protocol and the characters specifying the code. For some protocols there is an algorithm defined that compiles the characters into a real value.

**Example**

The code used by the SC5272 chip is named "sc5", has 3 data codes ('0', '1' and 'f') and a stop code ('S') defined.
So the textual representation may be "[sc5 0000f0000fffS]"


## Use the library

Using the library requires the following steps:

* A set of protocol definitions that you expect that they are used.
* A pin where the inbound signal comes in (receiver)
* A pin where the outbound signal can be send (sender)
* A function that get's called when a code was decoded.

For the receiver role the library uses a interrupt routine that gets called when ever a signal change on the pin has been detected.
Some microprocessors support only specidfic pins with interrupts
so please look up the documentation for **Arduino attachInterrupt()** function for the processor.

Sending a protocol uses no interrupts but also should not be interrupted by another ISR routine.


## The Implementation

There are 2 classes combined here:

**TabRF** -> SignalDecoder

The `TabRF` is an Arduino library that handles interrupt routines and the IO pins.
For receiving codes the Interrupt Routine only collects timings from the receiver IO pin and passes them to the SingnalParser for detection.
The sendig the timings are requested from the SingnalParser and send out to the sending pin.



Since the solutions of the manufacturers vary quiet a lot this library can be adapted to different protocols by registering the signal patterns in a definition table.
It has a modular approach so it can be adapted to other signal sources and frequencies.

To analyze the signals from senders the scanner example can be used to record the base timings. 



**SignalParser**

The `SignalParser` is a general usable class that knows all about the timing of codes in the protocol and knows how to decode and encode them. 


## The Wiring

```
+-------------+        +-------------+       +-------------+
|  RF433      |        |  Arduino /  |       |  RF433      |
|  receiver   |  -->   |  ESP8266    |  -->  |  sender     |  
|  module     |        |  board      |       |  module     |
+-------------+        +-------------+       +-------------+
```

The receiver modules that can be used must detect the RF signal and produce a signal when the carrier frequency has been detected. The polarity of the signal is not relevant.

The receivers I used and found reliable are the type RXB8 and RXB12, both with a ceramic resonator. The RF-5V and XY-MK-5V modules were not reliable in my environment.

The sender modules must produce a carrier frequency on HIGH output. When not transmitting a code the output is LOW so other devices can use the carrier frequency on their own.

I used several modules all with caramic resonators (no adjustable air coils). They seem to be less critical.


## RF Protocols

The protocols used with RF 433 signals all use specific carrier frequency on/off code patterns to send a specific information over the air.

Most protocols define code patterns for start and stop, LOW bits and HIGH bits but there are also protocols that use more than 2 code patterns.

To get a universal representation of a code sequence the protocol is given a short name and each code has a character. So any code
sequence corresponds to a textual representation like `it2 s_##___#____#_#__###_____#__#____x`

The first word is the name of the protocol registered in he table. The second word ist the sequence of codes.

The protocols are registered to the SignalParser Library.


## Signal decoding

The implementation of the library is made up of 3 parts:

* The TabRF library uses an interrupt routine and produces a stream of numbers that correlate to the time between 2 signal changes.
  
* The timings are delivered to a static ring buffer so the interrupt routine can return as soon as possible.

* By calling the rfsend.loop() function in the loop() function of the main sketch the signal timings are taken from the buffer and are passed to the SignalParser. 

* The TabRF library allows retrieving the last timings from the ring buffer and supports analyzing timings after receiving and recording them. 

* When a starting code is detected (a code that fits for a starting sequence) the capturing for this protocol starts.
  
* When receiving a timing that doesn't fit to any defined code the receiving is reset and will restart with a next starting code that is detected.

* When a stop code is detected or the max length of the protocol is reached the sequence is made available to the sketch using a callback function.

TODO:
A debug mode in the signalParser is enabled the callback function alos reports when a start sequence and some following data codes could be received but then parsing failed on a specific timing.

Analyzing Example


## Register a Protocol

To register a new protocol 2 structures are used.

The `Protocol` structure defines the characteristics on the protocol level. This is done using the method

    /**
    * @brief Register a new protocol.
    * @param name a short name for the protocol
    * @param minLen the minimal length of a valid code sequence.
    * @param maxLen the maximal length of a valid code sequence.
    * @param tolerance the tolerance in percent for timings to be recognized.
    * @param repeat the number of sequences to be sent in a row. 
    */
    uint8_t newProtocol(char *name, uint8_t minLen, uint8_t maxLen, uint8_t tolerance=4, uint8_t repeat=3);

The returned Protocol ID can be used to register the codes for the protocol.


## Register a Code

To register a new code for the processing the following function can be used:

up to 8 timings

uint8_t newCode(protID, ch, type, t1, t2, t3=0, t4=0, t5=0, t6=0, t7=0, t8=0); 


## Finding Codes on the internet

There are some good sources of protocol definitions available. Here are some links I found useful:

* <https://manual.pilight.org/protocols/433.92/index.html>
* <https://github.com/pilight/pilight/tree/master/libs/pilight/protocols/433.92>

https://www.arctech.com.tw/index.php/product/detail/293

arctech 



## Trimming the receiver 

On some receiver boards like the RxB8 you can observe that pulse transmissions often are rated longer than pulse pauses. By adjusting this in the timing some real better results can be observed and codes can be recognized better.

With the raw scanner (see scanner.md) you can collect some probes from a specific sender where you know the exact timings. I took a signal where 1:3 and 3:1 transmissions are used
and by subtracting a specific trim value from HIGH signal and adding them to LOW signal timings the exact 1:3 could be observed. You can define this optional parameter in the init  function.



void protocol(uint8_t minCodeLen, uint8_t maxCodeLen) 


  // minimal number of codes in a row required by the protocol
  uint8_t minCodeLen;

  // maximum number of codes in a row defining a complete  CodeSequence.
  uint8_t maxCodeLen;

  // tolerance of the timings in percent.
  uint8_t tolerance;

  // Number of repeats when sending.
  uint8_t sendRepeat;

  // Number of defined codes in this table
  uint8_t length;



 from a receiver and can send out    


receive and send signals patterns 


This is a RF signal encode and decode library that can be adapted to different protocols by specifying the timing conditions ia a table. Can be used with 433 MHz receivers and senders.

10.04.2018: This library was created on AVR and ported to esp8266 boards. It is still under construction.


## why RF signals sometime fail...

### Some noise from somewhere else

Here is an example next to a `[it2 s_##___#____#_#__###_____#____#__x]` sequence.
When receiving the 15. code the pause expected to have 1250 µsecs. was interrupted by another signal and ended therefore with about 409 µsecs.

```TXT
s 283	2658
_ 286  255 296 1269 	
# 286	1269 287  267 	
# 285	1268 289  264 	
_ 285	 249 289 1278 	
_ 286	 251 285 1282 	
_ 289	 249 287 1278 	
# 288	1268 291  261 	
_ 294	 239 294 1287 	
_ 287	 252 286 1281 	
_ 284	 254 287 1278 	
_ 290  251 286 1279 	
# 285	1259 294  268 	
_ 283	 259 287 1272 	
  291  409
```			

## TODO

* Add more protocols
* Port back to AVR.
* Use debugging output not using printf to save memory on AVR cores.
* Documentation on <https://www.mathertel.de/Arduino/TabRF.aspx>
