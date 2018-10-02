#include <Cosa/MCP2515.h>
#include <Cosa/UART.hh>
#include <Cosa/Trace.hh>

wlp::cosa::MCP2515 bus;

enum {
    Control = 0x0F,
    Mode = 0xE0,
    Config = 0x80,
};

void setup() {
    uart.begin(9600);
    trace.begin(&uart);

    trace << "Start" << endl;

    bus.reset();
    bus.modify_register(Control, Mode, Config);
    uint8_t mode = bus.read_register(Control) & Mode;
    trace << "Mode readback: " << mode << endl;
    trace << "Expected: " << (Config & Mode) << endl;
}

void loop() {
    delay(1000);
    trace << "Loop" << endl;
}
