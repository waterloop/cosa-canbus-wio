#include <Cosa/MCP2515.h>
#include <Cosa/UART.hh>
#include <Cosa/Trace.hh>
#include <MCP2515.h>

using namespace wlp;

static cosa::MCP2515 base;
static MCP2515 bus(&base);
static uint8_t buf[8];

void setup() {
    uart.begin(9600);
    trace.begin(&uart);
    while (bus.begin(CAN_500KBPS, MCP_16MHz) != Result::OK) {
        delay(100);
    }
    trace << "Sender started" << endl;
}

void loop() {
    bus.send_buffer(0x15, sizeof(buf), buf);
    int i = 7;
    while (i >= 0 && !++buf[i]) {
        --i;
    }
    delay(250);
}
