#include "MCP2515Rates.h"
#include <MCP2515.h>

using namespace wlp;

static uint8_t set_control_mode(MCP2515Base *base, uint8_t newMode) {
    base->modify_register(Register::Control, ControlMask::Mode, newMode);
    uint8_t mode = base->read_register(Register::Control) & ControlMask::Mode;
    return (mode == newMode) ? Result::OK : Result::Failed;
}

static uint8_t configure_rate(MCP2515Base *base, uint8_t canSpeed, uint8_t clockSpeed) {
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
    if (0 != eid) {
        buf[Bits::EIDL] = sid & 0xff;
        buf[Bits::EIDH] = sid >> 8;
        buf[Bits::SIDL] = eid & 0b11;
        buf[Bits::SIDL] |= (eid & 0x11100) << 3;
        buf[Bits::SIDL] |= Mask::ExtendedID;
        buf[Bits::SIDH] = eid >> 5;
    } else {
        buf[Bits::SIDL] = (sid & 0b111) << 5;
        buf[Bits::SIDH] = sid >> 3;
        buf[Bits::EIDL] = 0;
        buf[Bits::EIDH] = 0;
    }
    base->set_registers(address, buf, 4);
}

static void init_buffers(MCP2515Base *base) {
    write_id(base, Register::RXM0SIDH, 0);
    write_id(base, Register::RXM1SIDH, 0);
    write_id(base, Register::RXF0SIDH, 0);
    write_id(base, Register::RXF1SIDH, 0);
    write_id(base, Register::RXF2SIDH, 0);
    write_id(base, Register::RXF3SIDH, 0);
    write_id(base, Register::RXF4SIDH, 0);
    write_id(base, Register::RXF5SIDH, 0);

    for (uint8_t i = 0; i < Limit::TXBufferLength - 1; i++) {
        base->set_register(Register::TXB0CTRL + i, 0);
        base->set_register(Register::TXB1CTRL + i, 0);
        base->set_register(Register::TXB2CTRL + i, 0);
    }
    base->set_register(Register::RXB0CTRL, 0);
    base->set_register(Register::RXB1CTRL, 0);
}

static uint32_t read_id(MCP2515Base *base, uint8_t address) {
    uint32_t id;
    uint8_t buf[4];
    base->read_registers(address, buf, 4);
    id = (buf[Bits::SIDH] << 3) + (buf[Bits::SIDL] >> 5);
    if (buf[Bits::SIDL] & Mask::ExtendedID) {
        id = (id << 2) + (buf[Bits::SIDL] & 0b00000011);
        id <<= 16;
        id = id + (buf[Bits::EIDH] << 8) + buf[Bits::EIDL];
    }
    return id;
}

uint8_t MCP2515::begin(uint8_t canSpeed, uint8_t clockSpeed) {
    m_base->reset();
    uint8_t res = set_control_mode(m_base, Mode::Config);
    if (Result::OK != res) {
        return Result::Failed;
    }
    if (Result::OK != configure_rate(m_base, canSpeed, clockSpeed)) {
        return Result::Failed;
    }
    init_buffers(m_base);
    m_base->modify_register(
            Register::RXB0CTRL,
            RXControlMask::AcceptAny | RXControlMask::AcceptAnyID,
            RXControlMask::AcceptAny | RXControlMask::AcceptBUKT);
    m_base->modify_register(
            Register::RXB1CTRL,
            RXControlMask::AcceptAny,
            RXControlMask::AcceptAnyID);
    res = set_control_mode(m_base, Mode::Normal);
    if (Result::OK != res) {
        return Result::Failed;
    }
    return Result::OK;
}

uint8_t MCP2515::set_filter(uint8_t filterNumber, uint32_t filter) {
    uint8_t res = Result::OK;
    res = set_control_mode(m_base, Mode::Config);
    if(res > 0) {
        return Result::Failed;
    }
    if (filterNumber < 0 || filterNumber > 5) {
        return Result::Failed;
    } else if (filterNumber < 3) {
        write_id(m_base, Register::RXF0SIDH + 0x04 * filterNumber, filter);
    } else {
        write_id(m_base, Register::RXF3SIDH + 0x04 * (filterNumber - 3), filter);
    }
    res = set_control_mode(m_base, Mode::Normal);
    if(res != Result::OK) {
        return res;
    }
    return Result::OK;
}

uint8_t MCP2515::set_mask(uint8_t maskNumber, uint32_t mask) {
    uint8_t res = Result::OK;
    res = set_control_mode(m_base, Mode::Config);
    if (res > 0) {
        return Result::Failed;
    }
    if (maskNumber == 0) {
        write_id(m_base, Register::RXM0SIDH, m_id);
    } else if (maskNumber == 1) {
        write_id(m_base, Register::RXM1SIDH, m_id);
    } else {
        return Result::Failed;
    }
    res = set_control_mode(m_base, Mode::Normal);
    if (res != Result::OK) {
        return res;
    }
    return Result::OK;
}

uint8_t MCP2515::send_buffer(uint32_t id, uint8_t len, uint8_t *buf) {
    set_msg(id, len, buf);
    return send_msg();
}

void MCP2515::read_buffer(uint8_t len, uint8_t *buf) {
    read_msg();
    for (int i = 0; i < m_dataLength && i < len; ++i) {
        buf[i] = m_messageData[i];
    }
}

uint8_t MCP2515::get_error() {
    uint8_t eflg = m_base->read_register(Register::ErrorFlag);
    return (eflg & ErrorMask::Any) ? Error::ControlError : Error::None;
}

uint8_t MCP2515::get_message_status() {
    uint8_t res = m_base->read_status();
    return (res & StatusMask::RXInterruptMask)
        ? MessageState::MessagePending
        : MessageState::NoMessage;
}

uint32_t MCP2515::get_id() {
    return m_id;
}

void MCP2515::write_CAN_msg(uint8_t address) {
    m_base->set_registers(address + 5, m_messageData, m_dataLength);
    if (1 == m_remoteRequestFlag) {
        m_dataLength |= Mask::ExtendedID;
    }
    m_base->set_register(address + 4, m_dataLength);
    write_id(m_base, address, m_id);
}

void MCP2515::read_CAN_msg(uint8_t bufferSidhAddr) {
    uint8_t mcpAddr = bufferSidhAddr;
    uint8_t ctrl;
    m_id = read_id(m_base, mcpAddr);

    ctrl = m_base->read_register(mcpAddr - 1);
    m_dataLength = m_base->read_register(mcpAddr + 4);

    if (ctrl & 0x08) {
        m_remoteRequestFlag = 1;
    } else {
        m_remoteRequestFlag = 0;
    }

    m_dataLength &= Mask::DLC;
    m_base->read_registers(mcpAddr + 5, m_messageData, m_dataLength);
}

void MCP2515::start_transmit(uint8_t mcpAddr) {
    m_base->modify_register(
            mcpAddr - 1,
            TXControlMask::RequestInProcess,
            TXControlMask::RequestInProcess);
}

uint8_t MCP2515::get_next_free_buf(uint8_t *txBuf) {
    uint8_t res, ctrlval;
    uint8_t ctrlregs[Limit::TXBuffers] = {
        Register::TXB0CTRL,
        Register::TXB1CTRL,
        Register::TXB2CTRL,
    };
    *txBuf = 0x00;
    for (int i = 0; i < Limit::TXBuffers; i++) {
        ctrlval = m_base->read_register(ctrlregs[i]);
        if (!(ctrlval & TXControlMask::RequestInProcess)) {
            *txBuf = ctrlregs[i]+1;
            return Result::OK;
        }
    }
    return Result::AllBuffersBusy;
}

void MCP2515::set_msg(uint32_t id, uint8_t len, uint8_t *data) {
    m_id = id;
    m_dataLength = len;
    for (int i = 0; i < len; ++i) {
        m_messageData[i] = data[i];
    }
}

void MCP2515::clear_msg() {
    for (int i = 0; i < m_dataLength; ++i) {
        m_messageData[i] = 0x00;
    }
    m_id = 0;
    m_dataLength = 0;
    m_remoteRequestFlag = 0;
    m_filterFlag = 0;
}

uint8_t MCP2515::read_msg() {
    uint8_t status = m_base->read_status();
    uint8_t res;
    if (status & Status::RX0InterruptFired) {
        read_CAN_msg(Buffer::RX0);
        m_base->modify_register(Register::InterruptFlag, InterruptFlag::RX0, 0);
        return MessageState::MessageFetched;
    } else if (status & Status::RX1InterruptFired) {
        read_CAN_msg(Buffer::RX1);
        m_base->modify_register(Register::InterruptFlag, InterruptFlag::RX1, 0);
        return MessageState::MessageFetched;
    }
    return MessageState::NoMessage;
}

uint8_t MCP2515::send_msg() {
    uint8_t res, txBuf;
    uint16_t timeout = 0;
    do {
        res = get_next_free_buf(&txBuf);
        ++timeout;
    } while (
        res == Result::AllBuffersBusy &&
        timeout < Limit::AwaitBufferTimeout);
    if (timeout >= Limit::AwaitBufferTimeout) {
        return Result::AwaitBufferTimedOut;
    }
    timeout = 0;
    write_CAN_msg(txBuf);
    start_transmit(txBuf);
    do {
        ++timeout;
        res = m_base->read_register(txBuf);
    } while((res & 0x08) && (timeout < Limit::AwaitBufferTimeout));

    if (timeout >= Limit::AwaitBufferTimeout) {
        return Result::SendTimedOut;
    } else {
        return Result::OK;
    }
}
