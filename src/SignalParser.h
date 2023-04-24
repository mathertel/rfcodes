/**
 * @file: SignalParser.h
 *
 * This file is part of the RFCodes library that implements receiving an sending
 * RF and IR protocols.
 *
 * @copyright Copyright (c) by Matthias Hertel, https://www.mathertel.de.
 *
 * This work is licensed under a BSD 3-Clause style license,
 * https://www.mathertel.de/License.aspx.
 *
 * @brief
 * This signal parser recognizes patterns in timing code sequences that are
 * defined by declarative tables.
 *
 * Changelog:
 * * 29.04.2018 created by Matthias Hertel
 * * 06.08.2018 const char send, allow for sending only.
 */

// .h

// This signal parser recognizes patterns in timing code sequences that are
// defined by declarative tables.

// * Define the pattern using newProtocol and newCode.
// * Register a callback function using attachCallback
// * Pass timing code values into the parse function.

// * 20.3.2021: parse every protocol independently


#ifndef SignalParser_H_
#define SignalParser_H_

// #include <cstdint>
// #include <cstdlib>
// #include <cstring >

#include "debugout.h"

#define NUL '\0'

#define MAX_TIMELENGTH 8  // maximal length of a code definition
#define MAX_CODELENGTH 8  // maximal number of code definitions per protocol

#define MAX_SEQUENCE_LENGTH 120                                   // maximal length of a code sequence
#define MAX_TIMING_LENGTH (MAX_TIMELENGTH * MAX_SEQUENCE_LENGTH)  // maximal number of timings in a sequence

#define PROTNAME_LEN 12  // maximal protocol name len including ending '\0'

class SignalParser {
public:
  // ===== Type definitions =====


  // use-cases of a defined code (start,data,end).
  typedef enum {
    START = 0x01,              // A valid start code type.
    DATA = 0x02,               // A code containing some information
    END = 0x04,                // This code ends a sequence
    ANYDATA = (START | DATA),  // A code with data can be used to start a sequence
    ANY = (DATA | END)         // A code with data that can end the sequence
  } CodeType;

  // timings are using CodeTime datatypes meaning µsecs.
  typedef unsigned int CodeTime;

  // The Code structure is used to hold a specific timing sequence used in the protocol.
  // This Structure includes also the current state information while receiving the code.
  struct Code {
    CodeType type;  // type of usage of code
    char name;      // single character name for this code used for the message string.

    CodeTime time[MAX_TIMELENGTH];  // ideal time of the code part.

    // These members will be calculated:

    int timeLength;  // number of timings for this code
    CodeTime total;       // total time in this code

    CodeTime minTime[MAX_TIMELENGTH];  // average time of the code part.
    CodeTime maxTime[MAX_TIMELENGTH];  // average time of the code part.

    // these fields reflect the current status of the code.
    int cnt;     // number of discovered timings.
    bool valid;  // is true while discovering and the code is still possible.
  };             // struct Code


  // The Protocol structure is used to hold the basic settings for a protocol.
  struct Protocol {
    // These members must be initialized for load():

    /** name of the protocol */
    char name[PROTNAME_LEN];

    /** minimal number of codes in a row required by the protocol. */
    unsigned int minCodeLen;

    // maximum number of codes in a row defining a complete CodeSequence.
    unsigned int maxCodeLen;

    // tolerance of the timings in percent.
    unsigned int tolerance;

    // Number of repeats when sending.
    unsigned int sendRepeat;

    CodeTime baseTime;
    CodeTime realBase;

    Code codes[MAX_CODELENGTH];

    // ===== These members are used while parsing:

    // Number of defined codes in this table
    int codeLength;
    char seq[MAX_SEQUENCE_LENGTH];
    int seqLen;
  };  // struct Protocol


  // Callback when a code sequence was detected.
  typedef void (*CallbackFunction)(const char *code);


  // ===== Functions =====

private:
  // ===== class variables =====

  /** Protocol table and related settings */
  Protocol **_protocol;
  int _protocolAlloc = 0;
  int _protocolCount = 0;

  CallbackFunction _callbackFunc;

  /** find protocol by name */
  Protocol *_findProt(char *name);

  /** find code by name */
  Code *_findCode(Protocol *p, char codeName);

  /** reset all codes in a protocol */
  void _resetCodes(Protocol *p);

  /** reset the whole protocol to start capturing from scratch. */
  void _resetProtocol(Protocol *p);

  /** use the callback function when registered using format <protocolname> <sequence> */
  void _useCallback(Protocol *p);

  /** check if the duration fits for the protocol */
  void _parseProtocol(Protocol *p, CodeTime duration);

  /** recalculate adjusted timings. */
  void _recalcProtocol(Protocol *protocol, CodeTime baseTime);


  // ===== public functions =====

public:
  /** attach a callback function that will get passed any new code. */
  void attachCallback(CallbackFunction newFunction);

  // return the number of send repeats that should occure.
  int getSendRepeat(char *name);

  /** parse a single duration.
   * @param duration check if this duration fits to any definitions.
   */
  void parse(CodeTime duration);

  /** compose the timings of a sequence by using the code table.
   * @param sequence textual representation using "<protocolname> <codes>".
   */
  void compose(const char *sequence, CodeTime *timings, int len);

  /** Load a protocol to be used. */
  void load(Protocol *protocol, CodeTime otherBaseTime = 0);

  // ===== debug helpers =====

  /** Send a summary of the current code-table to the output. */
  void dumpProtocol(Protocol *p) {
    TRACE_MSG("dump %08x", p);

    if (p) {
      // dump the Protocol characteristics
      RAW_MSG("Protocol '%s', min:%d max:%d tol:%02u rep:%d\n",
              p->name, p->minCodeLen, p->maxCodeLen, p->tolerance,
              p->sendRepeat);

      Code *c = p->codes;
      int cnt = p->codeLength;

      while (c && cnt) {
        RAW_MSG("  '%c' |", c->name);

        for (int n = 0; n < c->timeLength; n++) {
          RAW_MSG("%5d -%5d |", c->minTime[n], c->maxTime[n]);
        }  // for
        RAW_MSG("\n");

        c++;
        cnt--;
      }  // while
      RAW_MSG("\n");
    }  // if
  }    // dumpProtocol()

  /** Send a summary of the current code-table to the output. */
  void dumpTable() {
    for (int n = 0; n < _protocolCount; n++) {
      Protocol *p = _protocol[n];
      dumpProtocol(p);
    }  // for
  }    // dumpTable()
};     // class

#endif  // SignalParser_H_
