// signal_parser.h

// This signal parser recognizes patterns in timing code sequences that are defined in a table.

// * Define the pattern using newProtocol and newCode.
// * Register a callback function using attachCallback
// * Pass timing code values into the parse function.


#ifndef SIGNAL_PARSER_H_
#define SIGNAL_PARSER_H_

#include <cstdint>
#include <cstdlib>
#include <cstring >

#include "debugout.h"

#define NUL '\0'

#define MAX_CODELENGTH 8 // maximal length of a code definition
#define MAX_SEQUENCE_LENGTH 64 // maximal length of a code sequence


#define SP_START 0x01 // A valid start code type.
#define SP_DATA 0x02 // A code containing some information
#define SP_END 0x04 // This code ends a sequence
#define SP_ANY (SP_DATA | SP_END) // A code can follow starting codes


class SignalParser
{

public:
  // /**
  //  * Code types
  //  * flags that specify the allowed usage of a code
  //  */
  // enum class CodeType {
  //   ANY = 0x10, // A valid code type.
  //   START = 0x01, // This code can start a sequence
  //   END = 0x02 // This code can end a sequence
  // };

  typedef int CodeType;


  // The Code structure is used to hold the definition of the protocol,
  // the timings and the current state information while receiving the code.
  struct Code {
    uint8_t protId;

    CodeType type; // type of usage of code
    char name; // single character name for this code used for the message
        // string.
    uint8_t length; // number of timings for this code
    uint32_t minTime[MAX_CODELENGTH]; // average time of the code part.
    uint32_t maxTime[MAX_CODELENGTH]; // average time of the code part.

    // these fields reflect the current status of the code.
    uint8_t cnt; // number of discovered timings.
    uint8_t valid; // is true while discovering and the code is still possible.

  }; // struct Code


  // Protocol definition
  struct Protocol {
    /** numeric id of the protocol */
    uint8_t id;

    /** name of the protocol */
    char name[12];

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
  }; // struct Protocol

  typedef void (*CallbackFunction)(char *code);


private:
  // ===== class variables =====

  /** Protocol table and related settings */
  Protocol *_protocol;
  int _protocolCount = 0;

  /** Code table and related settings */
  Code *_code;
  int _codeCount = 0;


  // Parser variables

  char _seq[MAX_SEQUENCE_LENGTH]; // Sequence of received codes
  uint8_t _seq_length; // length of current recorded code sequence
  Protocol *_seq_protocol; // detected protocol

  CallbackFunction _callbackFunc;

  // ===== private core functions =====

  /** find protocol by ID */
  Protocol *_findProt(uint8_t protId)
  {
    Protocol *p = _protocol;
    int cnt = _protocolCount;

    while (cnt > 0) {
      if (p->id == protId)
        break;
      p++;
      cnt--;
    }
    return (cnt ? p : nullptr);
  } // _findProt()


  /** find protocol by name */
  Protocol *_findProt(char *name)
  {
    Protocol *p = _protocol;
    int cnt = _protocolCount;

    while (cnt > 0) {
      if (strcmp(name, p->name) == 0) break;
      p++;
      cnt--;
    }
    return (cnt ? p : nullptr);
  } // _findProt()


  /** find protocol by name */
  Code *_findCode(uint8_t protId, char codeName)
  {
    Code *c = _code;
    int cnt = _codeCount;

    while (c && cnt > 0) {
      if (c->protId == protId && c->name == codeName) break;
      c++;
      cnt--;
    }
    return (cnt ? c : nullptr);
  } // _findCode()


  /**
   * Reset all counters in the code-table to start next matching
   * and optional reset current received signals.
   */
  void _resetCode(bool all)
  {
    Code *c = _code;
    int cnt = _codeCount;
    while (c && cnt) {
      c->cnt = 0;
      c->valid = true;
      c++;
      cnt--;
    } // while

    if (all) {
      TRACE_MSG("--reset--all-");
      _seq_protocol = nullptr;
      _seq_length = 0;
      _seq[0] = NUL;
    } else {
      TRACE_MSG("--reset");
    } // if
  } // _resetCode()


  // ===== public functions =====

public:
  /** attach a callback function that will get passed any new code. */
  void attachCallback(CallbackFunction newFunction)
  {
    _callbackFunc = newFunction;
  } // attachCallback()


  /**
   * @brief Register a new protocol.
   * @param name a short name for the protocol
   * @param minLen the minimal length of a valid code sequence.
   * @param maxLen the maximal length of a valid code sequence.
   * @param tolerance the tolerance in percent for timings to be recognized.
   * @param repeat the number of sequences to be sent in a row. 
   */
  uint8_t newProtocol(char *name, uint8_t minLen, uint8_t maxLen, uint8_t tolerance = 25, uint8_t repeat = 3)
  {
    TRACE_MSG("new protocol %s", name);

    // get space for protocol definition
    _protocolCount += 1;
    _protocol = (Protocol *)realloc(_protocol, _protocolCount * sizeof(struct Protocol));

    // fill last one.
    Protocol *p = &_protocol[_protocolCount - 1];
    p->id = _protocolCount;
    strncpy(p->name, name, sizeof(p->name));
    p->name[sizeof(p->name) - 1] = NUL;
    p->minCodeLen = minLen;
    p->maxCodeLen = maxLen;
    p->tolerance = tolerance;
    p->sendRepeat = repeat;
    p->length = 0;
    return (_protocolCount);
  } // newProtocol


  void newCode(uint8_t protId, char ch, CodeType type, uint32_t t1, uint32_t t2, uint32_t t3 = 0, uint32_t t4 = 0)
  {
    TRACE_MSG("new code %c", ch);

    Protocol *p = _findProt(protId);
    if (!p) {
      ERROR_MSG("unregistered protId");

    } else {
      // get space for protocol definition
      _codeCount += 1;
      _code = (Code *)realloc(_code, _codeCount * sizeof(struct Code));

      // fill last one.
      Code *c = &_code[_codeCount - 1];
      uint8_t l = 0;
      uint16_t radius;

      c->protId = protId;
      c->name = ch;
      c->type = type;
      c->cnt = 0;
      c->valid = true;

      if (t1 > 0) {
        c->minTime[l++] = t1;
      }

      if (t2 > 0) {
        c->minTime[l++] = t2;
      }

      if (t3 > 0) {
        c->minTime[l++] = t3;
      }

      if (t4 > 0) {
        c->minTime[l++] = t4;
      }
      c->length = l;

      // calc min and max
      for (int i = 0; i < l; i++) {
        uint32_t t = c->minTime[i];
        radius = (t * p->tolerance) / 100;
        c->minTime[i] = t - radius;
        c->maxTime[i] = t + radius;
      } // for
    } // if

  } // newCode


  // check for a timing with the given duration fits into a code.
  // and when a code is complete, check for protocol conditions start end end.
  void parse(uint32_t *durations)
  {
    while (*durations) {
      if (*durations == 305) {
        TRACE_MSG("stop");
      }
      parse(*durations++);

    } // while
  }

  // return true when duration was aborting finding a start code.
  void parse(uint32_t duration)
  {
    Code *foundCode = nullptr; // a completed code sequence
    bool match = false;
    bool retryCandidate = false;

    TRACE_MSG("parse(%d)", duration);

    if (_protocol) {
      // search for all codes for a possible match at the end of the sequence

      Code *c = _code;
      int len = _codeCount;

      while (c && len) {
        if (c->valid) {
          // check if timing fits into this code
          int8_t i = c->cnt;

          if ((_seq_length == 0) && !(c->type & SP_START)) {
            // codes other than start codes are nor acceptable as a first code in the sequence.
            TRACE_MSG("check: %c: n/s", c->name);
            c->valid = false;

          } else if ((_seq_length > 0) && !(c->type & SP_ANY)) {
            // codes other than data and end codes are nor acceptable during receiving.
            TRACE_MSG("check: %c: n/a", c->name);
            c->valid = false;

          } else if ((duration < c->minTime[i]) || (duration > c->maxTime[i])) {
            // This code sequence is not matching
            TRACE_MSG("check: %c: failed", c->name);
            c->valid = false;

            if ((i > 0) && (_seq_length == 0)) {
              // reanalyze this duration as a first duration for starting.
              retryCandidate = true;
            }

          } else {
            TRACE_MSG("check: %c: fits", c->name);
            match = true;

            // this timing is matching
            c->cnt = i = i + 1;

            if (i == c->length) {
              // this pattern is matching and code is complete
              foundCode = c;
              _seq[_seq_length++] = c->name;
              _seq[_seq_length] = NUL;

              // start matching from the beginning with next timing
              _resetCode(false);
              break;
            } // if
          } // if
        } // if
        len--;
        c++;
      } // while
      TRACE_MSG("---");

      if (match)
        TRACE_MSG("match.");
      if (foundCode)
        TRACE_MSG("found code.");

      if (!match) {
        // no matching code timing found => reset everything.
        _resetCode(true);

        if (retryCandidate)
          parse(duration);

      } else {
        if (foundCode) {
          // a complete code sequence was found

          if (_seq_length == 1) {
            // The first found pattern defines the protocol to be scanned further
            if (!(foundCode->type & SP_START)) {
              // first code is not a valid starting code.
              _resetCode(true); // reset all code scanning

            } else {
              _seq_protocol = _findProt(foundCode->protId);
            }

            // } else if (((_seq_length > _seq_protocol->minCodeLen) && !(foundCode->type & SP_END)) || (_seq_length == _seq_protocol->maxCodeLen)) {
          } else if ((foundCode->type & SP_END) || (_seq_length == _seq_protocol->maxCodeLen)) {
            // this is the last code in sequence

            if ((_callbackFunc) && (_seq_length >= _seq_protocol->minCodeLen)) {
              // found !
              char buffer[64];
              snprintf(buffer, sizeof(buffer), "%s %s", _seq_protocol->name, _seq);
              _callbackFunc(buffer);
            }
            _resetCode(true);

          } else if ((_seq_length == MAX_SEQUENCE_LENGTH - 2)) {
            // no code candidate, max Length exceeded !
            _resetCode(true); // reset current protocol
          } // if

        } // if (foundcode)
      } // if (match)
    } // if
    TRACE_MSG("_seq: %s\n", _seq);
  } // parse()


  /** compose the timings of a sequence by using the code table.
 * @param sequence textual representation using "<protocolname> <codes>".
 * 
*/
  void compose(const char *sequence, uint32_t *timings, int len)
  {
    char protname[12];

    // copy to protname
    strncpy(protname, sequence, sizeof(protname));
    protname[11] = NUL;
    char *s = strchr(protname, ' ');
    if (s) *s = NUL;

    s = (char *)strchr(sequence, ' ');
    if (s) s++;

    Protocol *p = _findProt(protname);

    if (p && timings) {
      while (*s && len) {
        Code *c = _findCode(p->id, *s);
        if (c) {
          for (int i = 0; i < c->length; i++) {
            *timings++ = (c->minTime[i] + c->maxTime[i]) / 2;
          } // for
        }
        s++;
        timings++;
        len--;
      }
      *timings = 0;
    } // if
  } // compose()


  /** Send a summary of the current code-table to the Serial output. */
  void dumpTable()
  {
    Protocol *p = _protocol;
    int pCnt = _protocolCount;
    while (p && pCnt) {
      // dump the Protocol characteristics
      INFO_MSG("Protocol %2d '%s', min:%d max:%d tol:%02u rep:%d", p->id, p->name, p->minCodeLen, p->maxCodeLen, p->tolerance, p->sendRepeat);

      Code *c = _code;
      int cnt = _codeCount;
      while (c && cnt) {
        if (p->id == c->protId) {
          fprintf(stderr, "  '%c' |", c->name);

          for (int n = 0; n < c->length; n++) {
            fprintf(stderr, "%5d -%5d |", c->minTime[n], c->maxTime[n]);
          } // for
          fprintf(stderr, "\n");
        } // if

        c++;
        cnt--;
      } // while
      fprintf(stderr, "\n");

      p++;
      pCnt--;
    } // while

  } // dumpTable()
}; // class

#endif // SIGNAL_PARSER_H_
