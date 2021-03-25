// protocols.h

// This is a collection of protocol definitions used in the 433 MHz Band for remote controls and data transfers.

#ifndef SignalParser_PROTOCOLS_H_
#define SignalParser_PROTOCOLS_H_

#include "SignalParser.h"

/** Definition of the "older" intertechno protocol with fixed 12 bits of data */
SignalParser::Protocol it1 = {
    "it1", // .name =
    .minCodeLen = 1 + 12,
    .maxCodeLen = 1 + 12,

    25, // .tolerance
    3, // .sendRepeat
    .baseTime = 380, // base time in µsecs

    3, // # of codes
    .codes = {
        {SP_START, 'B', 2, {1, 31}},
        {SP_DATA, '0', 4, {1, 3, 3, 1}},
        {SP_DATA, '1', 4, {1, 3, 1, 3}}}

};

/** Definition of the "newer" intertechno protocol with  data */
SignalParser::Protocol it2 = {
    "it2", // .name =
    .minCodeLen = 34,
    .maxCodeLen = 48,

    25, // .tolerance
    10, // .sendRepeat
    .baseTime = 280, // base time in µsecs

    5, // # of codes
    .codes = {
        {SP_START, 's', 2, {1, 10}},
        {SP_DATA, '_', 4, {1, 1, 1, 5}},
        {SP_DATA, '#', 4, {1, 5, 1, 1}},
        {SP_DATA, 'D', 4, {1, 1, 1, 1}},
        {SP_END, 'x', 2, {1, 38}}}

};


#endif  // SignalParser_PROTOCOLS_H_

// End.
