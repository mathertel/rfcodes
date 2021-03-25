// SignalParser.h

// This signal parser recognizes patterns in timing code sequences that are
// defined in a table.

// * Define the pattern using newProtocol and newCode.
// * Register a callback function using attachCallback
// * Pass timing code values into the parse function.

// * 20.3.2021: parse every protocol indepenently


#ifndef SignalParser_H_
#define SignalParser_H_

#include <cstdint>
#include <cstdlib>
#include <cstring >

#include "debugout.h"

#define NUL '\0'

#define MAX_TIMELENGTH 8 // maximal length of a code definition
#define MAX_CODELENGTH 8 // maximal number of code definitions per protocol

#define MAX_SEQUENCE_LENGTH 120 // maximal length of a code sequence
#define MAX_TIMING_LENGTH (MAX_TIMELENGTH * MAX_SEQUENCE_LENGTH) // maximal number of timings in a sequence

#define PROTNAME_LEN 12 // maximal protocol name len including ending '\0'

#define SP_START 0x01 // A valid start code type.
#define SP_DATA 0x02 // A code containing some information
#define SP_END 0x04 // This code ends a sequence
#define SP_ANY (SP_DATA | SP_END) // A code can follow starting codes


class SignalParser
{
public:
  // ===== Type definitions =====

  // timings are using CodeTime datatypes meaning µsecs.
  typedef int CodeTime;

  // use-cases of a defined code (start,data,end).
  typedef int CodeType;

  // The Code structure is used to hold the definition of the protocol,
  // the timings and the current state information while receiving the code.
  struct Code {
    CodeType type; // type of usage of code
    char name; // single character name for this code used for the message
        // string.
    uint8_t timeLength; // number of timings for this code
    CodeTime time[MAX_TIMELENGTH]; // ideal time of the code part.

    // These members will be calculated:

    CodeTime minTime[MAX_TIMELENGTH]; // average time of the code part.
    CodeTime maxTime[MAX_TIMELENGTH]; // average time of the code part.

    // these fields reflect the current status of the code.
    uint8_t cnt; // number of discovered timings.
    uint8_t valid; // is true while discovering and the code is still possible.
  }; // struct Code

  // struct Protocol; // forward declaration

  // Protocol definition
  struct Protocol {
    // These members must be initialized for load():

    /** name of the protocol */
    char name[PROTNAME_LEN];

    /** minimal number of codes in a row required by the protocol. */
    uint8_t minCodeLen;

    // maximum number of codes in a row defining a complete  CodeSequence.
    uint8_t maxCodeLen;

    // tolerance of the timings in percent.
    uint8_t tolerance;

    // Number of repeats when sending.
    uint8_t sendRepeat;

    CodeTime baseTime;

    // Number of defined codes in this table
    uint8_t codelength;
    Code codes[MAX_CODELENGTH];

    // ===== These members are used while parsing:

    bool valid; // is true while discovering and the code is still possible.

    char seq[MAX_SEQUENCE_LENGTH];
    int seqLen;


  }; // struct Protocol


  typedef void (*CallbackFunction)(char *code);


  // ===== Functions =====


private:
  // ===== class variables =====

  bool _debugMode = false;

  /** Protocol table and related settings */
  Protocol *_protocol;
  int _protocolCount = 0;

  CallbackFunction _callbackFunc;

  // ===== private core functions =====


  /** find protocol by name */
  Protocol *_findProt(char *name)
  {
    Protocol *p = _protocol;
    int cnt = _protocolCount;

    while (cnt > 0) {
      if (strcmp(name, p->name) == 0)
        break;
      p++;
      cnt--;
    }
    return (cnt ? p : nullptr);
  } // _findProt()


  /** find protocol by name */
  Code *_findCode(Protocol *p, char codeName)
  {
    Code *c = p->codes;
    int cnt = p->codelength;

    while (c && cnt) {
      if (c->name == codeName)
        break;
      c++;
      cnt--;
    }
    return (cnt ? c : nullptr);
  } // _findCode()


  void _resetCodes(Protocol *p)
  {
    TRACE_MSG("  reset codes: %s", p->name);

    Code *c = p->codes;
    int cCnt = p->codelength;
    while (c && cCnt) {
      c->valid = true;
      c->cnt = 0;

      c++;
      cCnt--;
    }
  }

  void _resetProtocol(Protocol *p)
  {
    TRACE_MSG("  reset prot: %s", p->name);
    p->valid = true;
    p->seqLen = 0;
    p->seq[0] = NUL;
    _resetCodes(p);
  }


  // ===== public functions =====

public:
  /** By defining debugMode=true some more callbacks are created starting with '*'. */
  void init(bool debugMode = false)
  {
    _debugMode = debugMode;
  } // init


  /** attach a callback function that will get passed any new code. */
  void attachCallback(CallbackFunction newFunction)
  {
    _callbackFunc = newFunction;
  } // attachCallback()


  void strcpyProtname(char *target, const char *signal)
  {
    char *p = target;
    char *s = (char *)signal;
    int len = PROTNAME_LEN - 1;
    while (len && *s && (*s != ' ')) {
      *p++ = *s++;
      len--;
    }
    *p = NUL;
  } // strcpyProtname


  // return the number of send repeats that should occure.
  int getSendRepeat(char *name)
  {
    Protocol *p = _findProt(name);
    return (p ? p->sendRepeat : 0);
  }


  // check for a timing with the given duration fits into a code.
  // and when a code is complete, check for protocol conditions start end end.
  void test(CodeTime *durations)
  {
    while (*durations) {
      if (*durations == 305) {
        TRACE_MSG("stop");
      }
      parse(*durations++);
      delay(1);
    } // while
  }


  void parseProtocol(Protocol *p, CodeTime duration)
  {
    Code *c = p->codes;
    int cCnt = p->codelength;
    bool anyValid = false;
    bool retryCandidate = false;

    while (c && cCnt) {

      if (c->valid) {
        // check if timing fits into this code
        int8_t i = c->cnt;
        CodeType type = c->type;
        bool matched = false; // until found that the new duration fits

        TRACE_MSG("check: %c:", c->name);

        if ((p->seqLen == 0) && !(type & SP_START)) {
          // codes other than start codes are nor acceptable as a first code
          // in the sequence.
          TRACE_MSG("  not start");

        } else if ((p->seqLen > 0) && !(type & SP_ANY)) {
          // codes other than data and end codes are nor acceptable during
          // receiving.
          TRACE_MSG("  not data");

        } else if ((duration < c->minTime[i]) || (duration > c->maxTime[i])) {
          // This timing is not matching.
          TRACE_MSG("  no fitting timing");

          if ((i > 0) && (p->seqLen == 0)) {
            // reanalyze this duration as a first duration for starting.
            retryCandidate = true;
            TRACE_MSG("  --retry");
          }

        } else {
          TRACE_MSG("  fits.");
          matched = true; // this code matches
        } // if

        // write back to code
        c->valid = matched;

        anyValid = anyValid || matched;

        if (retryCandidate) {
          // reset this code only and try again.
          TRACE_MSG("  start retry...");
          _resetProtocol(p);


        } else if (matched) {
          // this timing is matching
          TRACE_MSG("  matched.");
          c->cnt = i = i + 1;

          if (i == c->timeLength) {
            TRACE_MSG("  add '%c'", c->name);
            // all timings received so add code-character.
            p->seq[p->seqLen++] = c->name;
            p->seq[p->seqLen] = NUL;
            _resetCodes(p); // reset all codes but not the protocol

            if ((type & SP_END) || (p->seqLen == p->maxCodeLen)) {
              INFO_MSG(" found: %s\n", p->seq);
              _callbackFunc(p->seq);
              _resetProtocol(p);
            }
            break; // no more code checking in this protocol
          } // if
        }
      } // if (c->valid)

      if (retryCandidate) {
        // only loop once
        retryCandidate = false;
      } else {
        // next code
        c++;
        cCnt--;
      }
    } // while

    p->valid = anyValid;
    if (!anyValid) {
      TRACE_MSG("  no valid code any more.");
      _resetProtocol(p);
    }
    TRACE_MSG(" state(%s): %d %s\n", p->name, p->valid, p->seq);
  }


  // return true when duration was aborting finding a start code.
  void parse(CodeTime duration)
  {
    TRACE_MSG("parse(%d)", duration);

    Protocol *p = _protocol;
    int pLen = _protocolCount;

    // search for all codes for a possible match at the end of the sequence
    // try every protocol independently
    while (p && pLen) {

      if (p->valid)
        parseProtocol(p, duration);

      p++;
      pLen--;
    }
  } // parse()


  /** compose the timings of a sequence by using the code table.
   * @param sequence textual representation using "<protocolname> <codes>".
   *
   */
  void compose(const char *sequence, CodeTime *timings, int len)
  {
    char protname[12];

    // copy to protname
    strncpy(protname, sequence, sizeof(protname));
    protname[11] = NUL;
    char *s = strchr(protname, ' ');
    if (s)
      *s = NUL;

    s = (char *)strchr(sequence, ' ');
    if (s)
      s++;

    Protocol *p = _findProt(protname);

    if (p && timings) {
      while (*s && len) {
        Code *c = _findCode(p, *s);
        if (c) {
          for (int i = 0; i < c->timeLength; i++) {
            *timings++ = (c->minTime[i] + c->maxTime[i]) / 2;
          } // for
        }
        s++;
        len--;
      }
      *timings = 0;
    } // if
  } // compose()


  /** Send a summary of the current code-table to the output. */
  void dumpProtocol(Protocol *p)
  {
    if (p) {
      // dump the Protocol characteristics
      RAW_MSG("Protocol '%s', min:%d max:%d tol:%02u rep:%d\n",
              p->name, p->minCodeLen, p->maxCodeLen, p->tolerance,
              p->sendRepeat);

      Code *c = p->codes;
      int cnt = p->codelength;

      while (c && cnt) {
        RAW_MSG("  '%c' |", c->name);

        for (int n = 0; n < c->timeLength; n++) {
          RAW_MSG("%5d -%5d |", c->minTime[n], c->maxTime[n]);
        } // for
        RAW_MSG("\n");

        c++;
        cnt--;
      } // while
      RAW_MSG("\n");
    } // if
  } // dumpProtocol()


  /** Send a summary of the current code-table to the output. */
  void dumpTable()
  {
    Protocol *p = _protocol;
    int pCnt = _protocolCount;
    while (p && pCnt) {
      dumpProtocol(p);
      p++;
      pCnt--;
    } // while
  } // dumpTable()


  /** Load a protocol to be used. */
  int load(Protocol *protocol)
  {
    if (protocol) {
      TRACE_MSG("loading protocol %s", protocol->name);

      // // some rules to verify the input
      // if ((minLen > maxLen) || (maxLen >= MAX_SEQUENCE_LENGTH)) {
      //   return (0);
      // } else {

      // get space for protocol definition
      _protocolCount += 1;
      _protocol = (Protocol *)realloc(_protocol,
                                      _protocolCount * sizeof(struct Protocol));

      // fill last one.
      Protocol *p = &_protocol[_protocolCount - 1];
      memcpy(p, protocol, sizeof(Protocol));

      // calc min and max
      for (int codeCount = 0; codeCount < p->codelength; codeCount++) {
        Code *c = p->codes + codeCount;
        CodeTime baseTime = p->baseTime;

        // calculate absolute timing boundaries
        for (int i = 0; i < c->timeLength; i++) {
          CodeTime t = baseTime * c->time[i];
          int radius = (t * p->tolerance) / 100;
          TRACE_MSG("== %d %d %d %d ", baseTime, c->time[i], t, radius);

          c->minTime[i] = t - radius;
          c->maxTime[i] = t + radius;
        } // for
      } // for

      _resetProtocol(p);

      return (_protocolCount);
    } // if
  } // load()

}; // class

#endif // SignalParser_H_
