#include <MCP2515.h>

using namespace wlp;

static uint8_t set_control_mode(MCP2515Base *base, uint8_t newMode) {
    base->modify_register(Register::Control, ControlMask::Mode, newMode);
    uint8_t mode = base->read_register(Register::Control) & ControlMask::Mode;
    return (mode == newMode) ? Result::OK : Result::Failed;
}

static uint8_t configure_rate(uint8_t canSpeed, uint8_t clockSpeed) {
    uint8_t *pRates;
    if (MCP_16MHz == clockSpeed) {
        if (canSpeed >= NUM_RATES(rates16)) {
            return Result::Failed;
        }
        pRates = rates16;
    } else {
        if (canSpeed >= NUM_RATES(rates8)) {
            return Result::Failed;
        }
        pRates = rates8;
    }
    pRates += canSpeed * 3;
    base->set_register(Register::RateConfig1, pRates[0]);
    base->set_register(Register::RateConfig2, pRates[1]);
    base->set_register(Register::RateConfig3, pRates[2]);
    return Result::OK;
}

static void write_id(MCP2515Base *base, uint8_t address, uint32_t id) {
    uint16_t sid = id & 0xffff;
    uint16_t eid = id >> 16;
    uint8_t buf[4];
    if (eid != 0) {
        buf[Bits::EIDL] = sid & 0xff;
        buf[Bits::EIDH] = sid >> 8;
        buf[Bits::SIDL] = eid & 0b11;
        buf[Bits::SIDL] |= (eic & 0x11100) << 3;
        buf[Bits::SIDH] = eid >> 5;
    } else {
    }
}

static void init_buffers(MCP2515Base *base) {

}

uint8_t MCP2515::begin(uint8_t canSpeed, uint8_t clockSpeed) {
    m_base->reset();
}

uint8_t MCP2515::set_filter(uint8_t num, uint32_t data) {
}

uint8_t MCP2515::set_mask(uint8_t num, uint32_t data) {
}

uint8_t MCP2515::send_buffer(uint32_t id, uint8_t len, uint8_t *buf) {
}

void MCP2515::read_buffer(uint8_t len, uint8_t *buf) {
}

uint8_t MCP2515::get_error() {
}

uint8_t MCP2515::get_message_status() {
}

uint32_t MCP2515::get_id() {
}
