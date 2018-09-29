# MCP2515 CAN-BUS Library
Arduino Cosa driver for MCP2515 CAN-BUS controller and
transceiver. Install with `wio install cosa-canbus`.

## Usage
Define a driver instance with `wlp::MCP2515 bus(Board::D10)`
specifying the Chip-Select (CS) pin. The driver supports
standard CAN-BUS bit-rates and both 8 MHz and 16 MHz.

```c++
// 8 MHz at 500k bitrate
bus.begin(CAN_500KBPS, MCP_8MHz)
// 16 MHz at 125k bitrate
bus.begin(CAN_125KBPS, MCP_16MHz)
```

Send messages with

```c++
bus.send_buffer(id, buffer_size, buffer);
```

Read messages with

```c++
if (MessageState::MessagePending == bus.get_message_state()) {
    bus.read_buffer(buffer_size, buffer);
}
```
