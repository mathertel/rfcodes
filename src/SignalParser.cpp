/**
 * @file SignalParser.cpp
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
 * Change History see SignalParser.h
 */

#include <Arduino.h>

#include "SignalParser.h"


// ===== private functions =====


/** find protocol by name */
SignalParser::Protocol *SignalParser::_findProt(char *name)
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


/** find code by name */
SignalParser::Code *SignalParser::_findCode(Protocol *p, char codeName)
{
  Code *c = p->codes;
  int cnt = p->codeLength;

  while (c && cnt) {
    if (c->name == codeName)
      break;
    c++;
    cnt--;
  }
  return (cnt ? c : nullptr);
} // _findCode()


/** reset all codes in a protocol */
void SignalParser::_resetCodes(Protocol *p)
{
  Code *c = p->codes;
  int cCnt = p->codeLength;
  while (c && cCnt) {
    c->valid = true;
    c->cnt = 0;

    c++;
    cCnt--;
  }
} // _resetCodes()


/** reset the whole protocol to start capturing from scratch. */
void SignalParser::_resetProtocol(Protocol *p)
{
  TRACE_MSG("  reset prot: %s", p->name);
  p->seqLen = 0;
  p->seq[0] = NUL;
  _resetCodes(p);
} // _resetProtocol()


/** use the callback function when registered using format <protocolname> <sequence> */
void SignalParser::_useCallback(Protocol *p)
{
  if (p && _callbackFunc) {
    String code;
    code += p->name;
    code += ' ';
    code += p->seq;
    _callbackFunc(code.c_str());
  }
} // _useCallback()


/** check if the duration fits for the protocol */
void SignalParser::_parseProtocol(Protocol *p, CodeTime duration)
{
  Code *c = p->codes;
  int cCnt = p->codeLength;
  bool anyValid = false;
  bool retryCandidate = false;

  while (c && cCnt) {

    if (c->valid) {
      // check if timing fits into this code
      int8_t i = c->cnt;
      CodeType type = c->type;
      bool matched = false; // until found that the new duration fits

      TRACE_MSG("check: %c", c->name);

      if ((p->seqLen == 0) && !(type & START)) {
        // codes other than start codes are nor acceptable as a first code in the sequence.
        // TRACE_MSG("  not start");

      } else if ((p->seqLen > 0) && !(type & ANY)) {
        // codes other than data and end codes are nor acceptable during receiving.
        // TRACE_MSG("  not data");

      } else if ((duration < c->minTime[i]) || (duration > c->maxTime[i])) {
        // This timing is not matching.
        // TRACE_MSG("  no fitting timing");

        if ((i == 1) && (p->seqLen == 0)) {
          // reanalyze this duration as a first duration for starting.
          retryCandidate = true;
          // TRACE_MSG("  --retry");
        }

      } else {
        matched = true; // this code matches
      }                 // if

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
          // all timings received so add code-character.
          p->seq[p->seqLen++] = c->name;
          p->seq[p->seqLen] = NUL;
          // DEBUG_ESP_PORT.print(c->name);
          TRACE_MSG("  add '%s'", p->seq);
          _resetCodes(p); // reset all codes but not the protocol

          if ((type == END) && (p->seqLen < p->minCodeLen)) {
            // End packet found but sequence was not started early enough
            TRACE_MSG("  end fragment: %s", p->seq);
            _resetProtocol(p);

          } else if ((type & END) && (p->seqLen >= p->minCodeLen)) {
            TRACE_MSG("  found-1: %s", p->seq);
            _useCallback(p);
            _resetProtocol(p);

          } else if ((p->seqLen == p->maxCodeLen)) {
            TRACE_MSG("  found-2: %s", p->seq);
            _useCallback(p);
            _resetProtocol(p);
          }
          break; // no more code checking in this protocol
        }        // if
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

  if (!anyValid) {
    TRACE_MSG("  no codes.");
    _resetProtocol(p);
  }
} // _parseProtocol()


// ===== public functions =====


/** attach a callback function that will get passed any new code. */
void SignalParser::attachCallback(CallbackFunction newFunction)
{
  _callbackFunc = newFunction;
} // attachCallback()


// return the number of send repeats that should occure.
int SignalParser::getSendRepeat(char *name)
{
  Protocol *p = _findProt(name);
  return (p ? p->sendRepeat : 0);
}

/** parse a single duration.
 * @param duration check if this duration fits to any definitions.
 */
void SignalParser::parse(CodeTime duration)
{
  TRACE_MSG("(%d)", duration);

  Protocol *p = _protocol;
  int pLen = _protocolCount;

  // search for all codes for a possible match at the end of the sequence
  // try every protocol independently
  while (p && pLen) {
    _parseProtocol(p, duration);
    p++;
    pLen--;
  }
} // parse()


/** compose the timings of a sequence by using the code table.
 * @param sequence textual representation using "<protocolname> <codes>".
 */
void SignalParser::compose(const char *sequence, CodeTime *timings, int len)
{
  char protname[PROTNAME_LEN];

  char *s = strchr(sequence, ' ');

  if (s) {
    // extract protname
    {
      char *tar = protname;
      const char *src = sequence;
      while (*src && (*src != ' ')) {
        *tar++ = *src++;
      }
      *tar = NUL;
    }
    Protocol *p = _findProt(protname);

    s++; // to start of code characters

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
  }
} // compose()

/** Load a protocol to be used. */
int SignalParser::load(Protocol *protocol)
{
  if (protocol) {
    TRACE_MSG("loading protocol %s", protocol->name);

    // get space for protocol definition
    _protocolCount += 1;
    _protocol = (Protocol *)realloc(_protocol,
                                    _protocolCount * sizeof(struct Protocol));

    // fill last one.
    Protocol *p = &_protocol[_protocolCount - 1];
    memcpy(p, protocol, sizeof(Protocol));

    CodeTime baseTime = p->baseTime;

    // calc min and max and codesLength
    int cl = 0;
    while ((cl < MAX_CODELENGTH) && (p->codes[cl].name)) {
      Code *c = &(p->codes[cl]);

      // calculate # of durations and absolute timing boundaries
      int tl = 0;
      while ((tl < MAX_TIMELENGTH) && (c->time[tl])) {
        CodeTime t = baseTime * c->time[tl];
        int radius = (t * p->tolerance) / 100;
        TRACE_MSG("== %d %d %d %d ", baseTime, c->time[i], t, radius);

        c->minTime[tl] = t - radius;
        c->maxTime[tl] = t + radius;
        tl++;
      } // while
      c->timeLength = tl;

      cl++;
    }                   // while
    p->codeLength = cl; // no need to specify codeLength

    // delete:
    // for (int codeCount = 0; codeCount < p->codeLength; codeCount++) {
    //   Code *c = p->codes + codeCount;

    //   // calculate absolute timing boundaries
    //   for (int i = 0; i < c->timeLength; i++) {
    //     CodeTime t = baseTime * c->time[i];
    //     int radius = (t * p->tolerance) / 100;
    //     TRACE_MSG("== %d %d %d %d ", baseTime, c->time[i], t, radius);

    //     c->minTime[i] = t - radius;
    //     c->maxTime[i] = t + radius;
    //   } // for
    // }   // for

    _resetProtocol(p);

    return (_protocolCount);
  } // if
} // load()

// End.