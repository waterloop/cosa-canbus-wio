#include <MCP2515.h>
#include <sys/mcp2515.h>
#include <stdio.h>

int main(void) {
    wlp::linux::MCP2515 base("/dev/spidev0.0", 10000000);
    wlp::MCP2515 bus(&base);
    while (bus.begin(CAN_500KBPS, MCP_8MHz) != Result::OK) {
        printf("Can init failed, retrying\n");
        sleep(1);
    }
    printf("Can Inited\n");
    uint8_t buf[8];
    while (true) {
        if (bus.get_message_state() == MessageState::MessagePending) {
            bus.read_buffer(8, buf);
            printf("Data from %d: ", bus.get_id());
            for (int i = 0; i < 8; ++i) {
                printf("%02x ", buf[i]);
            }
            printf("\n");
        }
        sleep(1);
    }
}
