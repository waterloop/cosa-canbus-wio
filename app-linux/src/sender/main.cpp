#include <sys/mcp2515.h>
#include <MCP2515.h>
#include <unistd.h>
#include <stdio.h>

using namespace wlp;

int main(void) {
    linux::MCP2515 base("/dev/spidev0.0", 10000000);
    base.begin();
    MCP2515 bus(&base);
    while (bus.begin(CAN_500KBPS, MCP_8MHz) != Result::OK) {
        printf("CAN init failed, retrying\n");
        sleep(1);
    }
    printf("CAN inited\n");

    uint8_t buf[8];
    while (true) {
        bus.send_buffer(0x15, sizeof(buf), buf);
        sleep(0.5);
        int i = 7;
        while (i >= 0 && !++buf[i]) {
            --i;
        }
    }
}
