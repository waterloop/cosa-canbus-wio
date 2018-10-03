#include <MCP2515.h>
#include <unistd.h>
#include <stdio.h>
#include <assert.h>

using namespace wlp;

class MCP2515Test : public MCP2515Base{
public:
    void reset(void) override;
    uint8_t read_register(uint8_t address) override;
    void set_register(uint8_t address, uint8_t value) override;
    void modify_register(uint8_t address, uint8_t mask, uint8_t data) override;
    void set_registers(uint8_t address, uint8_t values[], uint8_t n) override;

    uint8_t read_status(void) override { assert(false); }
    void read_registers(uint8_t address, uint8_t values[], uint8_t n) override { assert(false); }

private:
    uint8_t m_regs[256];
};

void MCP2515Test::reset(void) {
    printf("[INFO] Reset\n");
}

uint8_t MCP2515Test::read_register(uint8_t address) {
    printf("[INFO] Read %02x\n", address);
    return m_regs[address];
}

void MCP2515Test::set_register(uint8_t address, uint8_t value) {
    printf("[INFO] Set %02x -> %02x\n", address, value);
    m_regs[address] = value;
}

void MCP2515Test::modify_register(uint8_t address, uint8_t mask, uint8_t data) {
    printf("[INFO] Modify %02x -> %02x & %02x\n", address, data, mask);
    m_regs[address] = data & mask;
}

void MCP2515Test::set_registers(uint8_t address, uint8_t values[], uint8_t n) {
    printf("[INFO] Set many %02x + %d\n", address, n);
    for (uint8_t i = 0; i < n; ++i) {
        m_regs[address + i] = values[i];
    }
}

int main(void) {
    MCP2515Test base;
    MCP2515 bus(&base);
    while (bus.begin(CAN_500KBPS, MCP_8MHz) != Result::OK) {
        printf("Can init failed, retrying\n");
        sleep(1);
    }
    printf("Can Inited\n");
}
