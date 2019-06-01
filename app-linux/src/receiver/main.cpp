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

    base.setup_interrupt(25);

    uint8_t buf[8];
    while (true) {
        base.wait_interrupt(500);
        while (bus.get_message_status() == MessageState::MessagePending) {
            bus.read_buffer(8, buf);
            printf("Data from %d\n", bus.get_id());
            for (int i = 0; i < 8; ++i) {
                printf("%02x ", buf[i]);
            }
            printf("\n");
        }
    }
}
