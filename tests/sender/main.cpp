#include "MCP2515.h"

#include "Cosa/Trace.hh"
#include "Cosa/UART.hh"

using namespace wlp;

MCP2515 bus(Board::D10);

void setup() {
    uart.begin(115200);
    trace.begin(&uart);

    while(bus.begin(CAN_500KBPS) != Result::OK) {
        trace << "CAN init failed, retrying" << endl;
        delay(100);
    }
    trace << "CAN init OK" << endl;
}

unsigned char buf[8] = {0};

void inc(uint8_t buf[8]) {
    int i = 7;
    while (i >= 0 && !++buf[i])
        --i;
}

void loop() {
    bus.send_buffer(0x00, sizeof(buf)/sizeof(buf[0]), buf);
    trace << "Sent data:";
    for (int i = 0; i < 8; ++i) {
        trace << " " << buf[i];
    }
    trace << endl;
    inc(buf);
    delay(50);
}
