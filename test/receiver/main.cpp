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

uint8_t stmp[8];

void loop() {
    if (MessageState::MessagePending == bus.get_message_state()) {
        bus.read_buffer(8, stmp);
        trace << "Recevied data from " << bus.get_id() << ":";
        for (int i = 0; i < 8; ++i) {
            trace << " " << stmp[i];
        }
        trace << endl;
    }
}
