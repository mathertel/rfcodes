# Examples

The following examples are provided:

## Intertechno

This example shows how to receive and send the protocols used by the intertechno devices.
Here 2 different protocols can be decoded using the same receiver.

## TempSensor

This example is part of the TabRF library showing how to receive temperature+humidity from a cresta based sensor.
The protocol used can transport data and is using manchester coding.

See [Cresta protocol](../docs/cresta_protocol.md) for more details.


## Scanner

This is a standalone sketch that can record received timings around a specific condition.

See [Scanner documentation](../docs/scanner.md) for more details.

The default condition is waiting for a long transmission time (6000-12000 µsecs).

While recording all received timings the condition is evaluated and when the condition is detected some 512 more timings are recorded.
Then all timings are send to Serial output to be visible in the monitor and can be analyzed. 


