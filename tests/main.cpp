#include "MCP2515.h"

#include "Cosa/Trace.hh"
#include "Cosa/UART.hh"

MCP2515 bus(Board::D10);

void setup() {
    uart.begin(115200);
    trace.begin(&uart);

    while(bus.begin(CAN_500KBPS) != CAN_OK) {
        trace << "CAN init failed, retrying" << endl;
        delay(100);
    }
    trace << "CAN init OK" << endl;
}

unsigned char stmp[8] = {0, 1, 2, 3, 4, 5, 6, 7};

void loop() {
    bus.send_buffer(0x00, 0, 8, stmp);  
    trace << "Sent data" << endl;
    delay(1000);
}

