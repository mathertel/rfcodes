// protocols.h

// This is a collection of protocol definitions used in the 433 MHz Band for remote controls and data transfers.

#ifndef SignalParser_PROTOCOLS_H_
#define SignalParser_PROTOCOLS_H_

#include "SignalParser.h"

/** namespace for defining codes for the Arduino RFCodes library. */
namespace RFCodes
{

/** Definition of the "older" intertechno protocol with fixed 12 bits of data */
SignalParser::Protocol it1 = {
    "it1",
    .minCodeLen = 1 + 12,
    .maxCodeLen = 1 + 12,

    .tolerance = 25,
    .sendRepeat = 4,
    .baseTime = 400,
    .codes = {
        {SignalParser::CodeType::START, 'B', {1, 31}},
        {SignalParser::CodeType::DATA, '0', {1, 3, 3, 1}},
        {SignalParser::CodeType::DATA, '1', {1, 3, 1, 3}}}

};


/** Definition of the "newer" intertechno protocol with 32 - 46 data bits data */
SignalParser::Protocol it2 = {
    "it2", // .name =
    .minCodeLen = 34,
    .maxCodeLen = 48,

    .tolerance = 25,
    .sendRepeat = 10,
    .baseTime = 280, // base time in µsecs
    .codes = {
        {SignalParser::CodeType::START, 's', {1, 10}},
        {SignalParser::CodeType::DATA, '_', {1, 1, 1, 5}},
        {SignalParser::CodeType::DATA, '#', {1, 5, 1, 1}},
        {SignalParser::CodeType::DATA, 'D', {1, 1, 1, 1}},
        {SignalParser::CodeType::END, 'x', {1, 38}}}

};


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


/** Definition of the protocol from ev1527 and similar chips with 20 address and 4 data bits. */
SignalParser::Protocol ev1527 = {
    "ev1527",
    .minCodeLen = 1 + 20 + 4,
    .maxCodeLen = 1 + 20 + 4,

    .tolerance = 25,
    .sendRepeat = 3,
    .baseTime = 320,
    .codes = {
        {SignalParser::CodeType::START, 's', {1, 31}},
        {SignalParser::CodeType::DATA, '0', {1, 3}},
        {SignalParser::CodeType::DATA, '1', {3, 1}}
      }};


/** register the cresta protocol with a length of 59 codes; used for sensor data transmissions.
 * See /docs/cresta_protocol.h */
SignalParser::Protocol cw = {
    "cw",
    .minCodeLen = 59,
    .maxCodeLen = 59,

    .tolerance = 16,
    .sendRepeat = 3,
    .baseTime = 500,
    .codes = {
        {SignalParser::CodeType::START, 'H', {2, 2, 2, 2, 2}},
        {SignalParser::CodeType::DATA, 's', {1, 1}},
        {SignalParser::CodeType::DATA, 'l', {2}}}};

} // namespace RFCodes

#endif // SignalParser_PROTOCOLS_H_

// End.
