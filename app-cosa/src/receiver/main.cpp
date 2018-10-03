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
    trace << "Receiver started" << endl;
}

void loop() {
    if (bus.get_message_status() == MessageState::MessagePending) {
        bus.read_buffer(8, buf);
        trace << "Data from " << bus.get_id() << endl;
        for (int i = 0; i < 8; ++i) {
            trace << buf[i] << " ";
        }
        trace << endl;
    }
}
