#include <sys/mcp2515.h>
#include <stdio.h>

wlp::linux::MCP2515 bus("/dev/spidev0.0", 10000000);

enum {
    Control = 0x0F,
    Mode = 0xE0,
    Config = 0x80,
};

int main(void) {
    bus.begin();
    bus.reset();
    bus.modify_register(Control, Mode, Config);
    uint8_t mode = bus.read_register(Control) & Mode;
    printf("Mode readback: %d\n", mode);
    printf("Expected: %d\n", Config & Mode);
}
