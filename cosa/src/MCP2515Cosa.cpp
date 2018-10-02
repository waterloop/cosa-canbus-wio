#include <Cosa/MCP2515.h>

using namespace wlp;

cosa::MCP2515::MCP2515(Board::DigitalPin cs) :
    SPI::Driver(cs) {}

void cosa::MCP2515::reset(void) {
    spi.acquire(this);
    spi.begin();
    spi.transfer(Instruction::Reset);
    spi.end();
    spi.release();
}

uint8_t cosa::MCP2515::read_status(void) {
    uint8_t tx[2] = {Instruction::ReadStatus, Instruction::Fetch};
    uint8_t rx[2] = {0, 0};
    spi.acquire(this);
    spi.begin();
    spi.transfer(rx, tx, 2);
    spi.end();
    spi.release();
    return rx[1];
}

uint8_t cosa::MCP2515::read_register(uint8_t address) {
    uint8_t tx[3] = {Instruction::Read, address, Instruction::Fetch};
    uint8_t rx[3] = {0, 0, 0};
    spi.acquire(this);
    spi.begin();
    spi.transfer(rx, tx, 3);
    spi.end();
    spi.release();
    return rx[2];
}

void cosa::MCP2515::read_registers(uint8_t address, uint8_t values[], uint8_t n) {
    uint8_t tx[2] = {Instruction::Read, address};
    spi.acquire(this);
    spi.begin();
    spi.transfer(tx, 2);
    spi.read(values, n);
    spi.end();
    spi.release();
}

void cosa::MCP2515::set_register(uint8_t address, uint8_t value) {
    uint8_t tx[3] = {Instruction::Write, address, value};
    spi.acquire(this);
    spi.begin();
    spi.transfer(tx, 3);
    spi.end();
    spi.release();
}

void cosa::MCP2515::set_registers(uint8_t address, uint8_t values[], uint8_t n) {
    uint8_t tx[2] = {Instruction::Write, address};
    spi.acquire(this);
    spi.begin();
    spi.transfer(tx, 2);
    spi.transfer(values, n);
    spi.end();
    spi.release();
}

void cosa::MCP2515::modify_register(uint8_t address, uint8_t mask, uint8_t data) {
    uint8_t tx[4] = {Instruction::Modify, address, mask, data};
    spi.acquire(this);
    spi.begin();
    spi.transfer(tx, 4);
    spi.end();
    spi.release();
}
