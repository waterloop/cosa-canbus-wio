#include "MCP2515.h"

using namespace wlp;

MCP2515::MCP2515(Board::DigitalPin cs_pin)
    : SPI::Driver(cs_pin) {}

// PUBLIC

uint8_t MCP2515::begin(const uint8_t bus_rate) {
    this->reset();

    uint8_t res = set_control_mode(Mode::Config);
    if (res != Result::OK) {
        #if DEBUG_MODE
        trace << "Switching to config mode failed\n";
        #endif
        return Result::Failed;
    }
    #if DEBUG_MODE
        trace << "Config mode entered successfully\n";
    #endif

    if (this->configure_rate(bus_rate) != Result::OK) {
        #if DEBUG_MODE
        trace << "Setting transmission rate failed\n";
        #endif
        return Result::Failed;
    }
    #if DEBUG_MODE
    trace << "Transmission rate set successfully\n";
    #endif

    this->init_buffers();
    // Enable interrupts on receipt
    set_register(Register::InterruptEnable,
                 InterruptFlag::RX0 | InterruptFlag::RX0);

    // Receive messages from all sources
    modify_register(Register::RXB0CTRL,
                    RXControlMask::AcceptAny | RXControlMask::AcceptAnyID,
                    RXControlMask::AcceptAny | RXControlMask::AcceptBUKT);
    modify_register(Register::RXB1CTRL, RXControlMask::AcceptAny,
                    RXControlMask::AcceptAnyID);

    res = set_control_mode(Mode::Normal);
    if (res != Result::OK) {
        #if DEBUG_MODE
        trace << "Retuning to normal mode failed\n";
        #endif
        return Result::Failed;
    }

    #if DEBUG_MODE
    trace << "Returned to normal mode\n";
    #endif
    return Result::OK;
}

uint8_t MCP2515::set_mask(uint8_t mask_number, uint32_t mask) {

    uint8_t res = Result::OK;
    #if DEBUG_MODE
    trace << "Starting mask setup\n";
    #endif
    res = set_control_mode(Mode::Config);
    if (res > 0) {
        #if DEBUG_MODE
        trace << "Switching to config mode failed\n";
        #endif
        return Result::Failed;
    }

    if (mask_number == 0)
        write_id(Register::RXM0SIDH, id);
    else if(mask_number == 1)
        write_id(Register::RXM1SIDH, id);
    else
        return Result::Failed;

    res = set_control_mode(Mode::Normal);
    if (res != Result::OK) {
        #if DEBUG_MODE
        trace << "Returning to normal model failed\nMask setup failed\n";
        #endif
        return res;
    }
    #if DEBUG_MODE
    trace << "Mask setup successfull\n";
    #endif

    return Result::OK;
}

uint8_t MCP2515::set_filter(uint8_t filter_number, uint32_t filter)
{
    uint8_t res = Result::OK;
    #if DEBUG_MODE
    trace << "Starting filter setup\n";
    #endif
    res = set_control_mode(Mode::Config);
    if(res > 0) {
        #if DEBUG_MODE
        trace << "Switching to config mode failed\n";
        #endif
        return Result::OK;
    }

    switch(filter_number) {
        case 0:
            write_id(Register::RXF0SIDH, filter);
        break;

        case 1:
            write_id(Register::RXF1SIDH, filter);
        break;

        case 2:
            write_id(Register::RXF2SIDH, filter);
        break;

        case 3:
            write_id(Register::RXF3SIDH, filter);
        break;

        case 4:
            write_id(Register::RXF4SIDH, filter);
        break;

        case 5:
            write_id(Register::RXF5SIDH, filter);
        break;

        default:
        res = Result::Failed;
    }

    res = set_control_mode(Mode::Normal);
    if(res != Result::OK) {
        #if DEBUG_MODE
        trace << "Returning to normal mode failed\nFilter setup failed\n";
        #endif
        return res;
    }

    #if DEBUG_MODE
    trace << "Filter setup successful\n";
    #endif

    return Result::OK;
}

uint8_t MCP2515::send_buffer(uint32_t id, uint8_t len, uint8_t *buf)
{
    set_msg(id, len, buf);
    return send_msg();
}

void MCP2515::read_buffer(uint8_t len, uint8_t *buf)
{
    this->read_msg();
    for (int i = 0; i < this->data_length && i < len; ++i)
        buf[i] = this->message_data[i];
}

uint8_t MCP2515::get_message_state(void) {
    uint8_t res = read_status();
    return (res & StatusMask::RXInterruptMask) ?
        MessageState::MessagePending : MessageState::NoMessage;
}

uint8_t MCP2515::get_error(void) {
    uint8_t eflg = read_register(Register::ErrorFlag);
    return (eflg & ErrorMask::Any) ? Error::ControlError : Error::None;
}

uint32_t MCP2515::get_id(void) {
    return this->id;
}

// PRIVATE

void MCP2515::reset(void)
{
    spi.acquire(this);
    spi.begin();
    spi.transfer(Instruction::Reset);
    spi.end();
    spi.release();
}

uint8_t MCP2515::read_register(uint8_t address)
{
    spi.acquire(this);
    spi.begin();
    spi.transfer(Instruction::Read);
    spi.transfer(address);
    uint8_t ret = spi.transfer(Instruction::Fetch);
    spi.end();
    spi.release();
    return ret;
}

void MCP2515::read_registers(uint8_t address, uint8_t values[], const uint8_t n)
{
    spi.acquire(this);
    spi.begin();
    spi.transfer(Instruction::Read);
    spi.transfer(address);
    for (uint8_t i = 0; i < n && i < Limit::MessageBufferLength; i++) {
        // The board will auto-increment address pointer
        values[i] = spi.transfer(Instruction::Fetch);
    }
    spi.end();
    spi.release();
}

void MCP2515::set_register(uint8_t address, const uint8_t value)
{
    spi.acquire(this);
    spi.begin();
    spi.transfer(Instruction::Write);
    spi.transfer(address);
    spi.transfer(value);
    spi.end();
    spi.release();
}

void MCP2515::set_registers(uint8_t address, uint8_t values[], const uint8_t n)
{
    spi.acquire(this);
    spi.begin();
    spi.transfer(Instruction::Write);
    spi.transfer(address);
    for (uint8_t i = 0; i < n && i < Limit::MessageBufferLength; i++) {
        // The board will auto-increment address pointer
        values[i] = spi.transfer(values[i]);
    }
    spi.end();
    spi.release();
}

void MCP2515::modify_register(uint8_t address, const uint8_t mask,
                              const uint8_t data)
{
    spi.acquire(this);
    spi.begin();
    spi.transfer(Instruction::Modify);
    spi.transfer(address);
    spi.transfer(mask);
    spi.transfer(data);
    spi.end();
    spi.release();
}

uint8_t MCP2515::read_status(void)
{
    spi.acquire(this);
    spi.begin();
    spi.transfer(Instruction::ReadStatus);
    uint8_t status = spi.transfer(Instruction::Fetch);
    spi.end();
    spi.release();
	return status;
}

uint8_t MCP2515::set_control_mode(const uint8_t new_mode)
{
    modify_register(Register::Control, ControlMask::Mode, new_mode);
    uint8_t mode = read_register(Register::Control) & ControlMask::Mode;
    return (mode == new_mode) ? Result::OK : Result::Failed;
}

uint8_t MCP2515::configure_rate(const uint8_t bus_rate)
{
    uint8_t set = 1, cfg1, cfg2, cfg3;
    switch (bus_rate)
    {
        case CAN_5KBPS:
            cfg1 = MCP_16MHz_5kBPS_CFG1;
            cfg2 = MCP_16MHz_5kBPS_CFG2;
            cfg3 = MCP_16MHz_5kBPS_CFG3;
        break;

        case CAN_10KBPS:
            cfg1 = MCP_16MHz_10kBPS_CFG1;
            cfg2 = MCP_16MHz_10kBPS_CFG2;
            cfg3 = MCP_16MHz_10kBPS_CFG3;
        break;

        case CAN_20KBPS:
            cfg1 = MCP_16MHz_20kBPS_CFG1;
            cfg2 = MCP_16MHz_20kBPS_CFG2;
            cfg3 = MCP_16MHz_20kBPS_CFG3;
        break;

        case CAN_40KBPS:
            cfg1 = MCP_16MHz_40kBPS_CFG1;
            cfg2 = MCP_16MHz_40kBPS_CFG2;
            cfg3 = MCP_16MHz_40kBPS_CFG3;
        break;

        case CAN_50KBPS:
            cfg1 = MCP_16MHz_50kBPS_CFG1;
            cfg2 = MCP_16MHz_50kBPS_CFG2;
            cfg3 = MCP_16MHz_50kBPS_CFG3;
        break;

        case CAN_80KBPS:
            cfg1 = MCP_16MHz_80kBPS_CFG1;
            cfg2 = MCP_16MHz_80kBPS_CFG2;
            cfg3 = MCP_16MHz_80kBPS_CFG3;
        break;

        case CAN_100KBPS:
            cfg1 = MCP_16MHz_100kBPS_CFG1;
            cfg2 = MCP_16MHz_100kBPS_CFG2;
            cfg3 = MCP_16MHz_100kBPS_CFG3;
        break;

        case CAN_125KBPS:
            cfg1 = MCP_16MHz_125kBPS_CFG1;
            cfg2 = MCP_16MHz_125kBPS_CFG2;
            cfg3 = MCP_16MHz_125kBPS_CFG3;
        break;

        case CAN_200KBPS:
            cfg1 = MCP_16MHz_200kBPS_CFG1;
            cfg2 = MCP_16MHz_200kBPS_CFG2;
            cfg3 = MCP_16MHz_200kBPS_CFG3;
        break;

        case CAN_250KBPS:
            cfg1 = MCP_16MHz_250kBPS_CFG1;
            cfg2 = MCP_16MHz_250kBPS_CFG2;
            cfg3 = MCP_16MHz_250kBPS_CFG3;
        break;

        case CAN_500KBPS:
            cfg1 = MCP_16MHz_500kBPS_CFG1;
            cfg2 = MCP_16MHz_500kBPS_CFG2;
            cfg3 = MCP_16MHz_500kBPS_CFG3;
        break;

        default:
        set = 0;
        break;
    }

    if (set) {
        set_register(Register::RateConfig1, cfg1);
        set_register(Register::RateConfig2, cfg2);
        set_register(Register::RateConfig3, cfg3);
        return Result::OK;
    } else {
        return Result::Failed;
    }
}

void MCP2515::init_buffers(void)
{
    write_id(Register::RXM0SIDH, 0);
    write_id(Register::RXM1SIDH, 0);
    write_id(Register::RXF0SIDH, 0);
    write_id(Register::RXF1SIDH, 0);
    write_id(Register::RXF2SIDH, 0);
    write_id(Register::RXF3SIDH, 0);
    write_id(Register::RXF4SIDH, 0);
    write_id(Register::RXF5SIDH, 0);

    // Clear the transmit buffers
    for (uint8_t i = 0; i < Limit::TXBufferLength - 1; i++) {
        set_register(Register::TXB0CTRL + i, 0);
        set_register(Register::TXB1CTRL + i, 0);
        set_register(Register::TXB2CTRL + i, 0);
    }

    set_register(Register::RXB0CTRL, 0);
    set_register(Register::RXB1CTRL, 0);
}

void MCP2515::write_id(const uint8_t address, const uint32_t id)
{
    uint16_t sid = id & 0x0000FFFF;
    uint16_t eid = (id & 0xFFFF0000) >> 16;
    uint8_t buf[4];

    if (eid != 0) {
        buf[Bits::EIDL] = sid & 0x00FF;
        buf[Bits::EIDH] = (sid & 0xFF00) >> 8;
        buf[Bits::SIDL] = eid & 0b00000011;
        buf[Bits::SIDL] |= (eid & 0x00011100) << 3;
        buf[Bits::SIDL] |= Mask::ExtendedID;
        buf[Bits::SIDH] = eid >> 5;
    } else {
        buf[Bits::SIDL] = (sid & 0b00000111) << 5;
        buf[Bits::SIDH] = sid >> 3;
        buf[Bits::EIDL] = 0;
        buf[Bits::EIDH] = 0;
    }

    set_registers(address, buf, 4);
}

uint32_t MCP2515::read_id(const uint8_t address)
{
    uint32_t id;
    uint8_t buf[4];

    read_registers(address, buf, 4);

    id = (buf[Bits::SIDH] << 3) + (buf[Bits::SIDL] >> 5);

    if (buf[Bits::SIDL] & Mask::ExtendedID) {
        id = (id << 2) + (buf[Bits::SIDL] & 0b00000011);
        id <<= 16;
        id = id + (buf[Bits::EIDH] << 8) + buf[Bits::EIDL];
    }
    return id;
}

void MCP2515::write_CAN_msg(const uint8_t address)
{
    set_registers(address + 5, this->message_data, this->data_length);
    if (this->remote_request_flag == 1)
        this->data_length |= Mask::ExtendedID;
    set_register((address+4), this->data_length);
    write_id(address, this->id);
}

void MCP2515::read_CAN_msg(const uint8_t buffer_sidh_addr)
{
    uint8_t mcp_addr = buffer_sidh_addr, ctrl;
    this->id = read_id(mcp_addr);

    ctrl = read_register(mcp_addr - 1);
    this->data_length = read_register(mcp_addr + 4);

    if ((ctrl & 0x08)) {
        this->remote_request_flag = 1;
    } else {
        this->remote_request_flag = 0;
    }

    this->data_length &= Mask::DLC;
    read_registers(mcp_addr + 5, this->message_data, this->data_length);
}

void MCP2515::start_transmit(const uint8_t mcp_addr)
{
    // Control register is offset 1 back
    modify_register(mcp_addr - 1, TXControlMask::RequestInProcess,
                    TXControlMask::RequestInProcess);
}

uint8_t MCP2515::get_next_free_buf(uint8_t *txbuf_n)
{
    uint8_t res, ctrlval;
    uint8_t ctrlregs[Limit::TXBuffers] = {
        Register::TXB0CTRL,
        Register::TXB1CTRL,
        Register::TXB2CTRL,
    };

    *txbuf_n = 0x00;

    // Check all transaction buffers
    for (int i = 0; i < Limit::TXBuffers; i++) {
        ctrlval = read_register(ctrlregs[i]);
        if (!(ctrlval & TXControlMask::RequestInProcess)) {
            *txbuf_n = ctrlregs[i]+1;
            return Result::OK;
        }
    }

    return Result::AllBuffersBusy;
}

void MCP2515::set_msg(uint32_t id, uint8_t len, uint8_t *data) {
    this->id = id;
    this->data_length = len;
    for (int i = 0; i < len; i++) {
        this->message_data[i] = data[i];
    }
}

void MCP2515::clear_msg() {
    for(int i = 0; i < this->data_length; i++) {
        this->message_data[i] = 0x00;
    }
    this->id = 0;
    this->data_length = 0;
    this->remote_request_flag = 0;
    this->filter_flag = 0;
}

uint8_t MCP2515::send_msg()
{
    uint8_t res, txbuf_n;
    uint16_t timeout = 0;

    do {
        res = this->get_next_free_buf(&txbuf_n);
        ++timeout;
    } while (res == Result::AllBuffersBusy &&
             (timeout < Limit::AwaitBufferTimeout));

    if(timeout >= Limit::AwaitBufferTimeout)
        return Result::AwaitBufferTimedOut;

    timeout = 0;
    this->write_CAN_msg(txbuf_n);
    this->start_transmit(txbuf_n);

    do {
        ++timeout;
        res = read_register(txbuf_n);  // This reads TX#CTRL
    } while((res & 0x08) && (timeout < Limit::AwaitBufferTimeout));

    if(timeout >= Limit::AwaitBufferTimeout)
        return Result::SendTimedOut;
    else
        return Result::OK;
}

uint8_t MCP2515::read_msg()
{
    uint8_t status = this->read_status(), res;

    if (status & Status::RX0InterruptFired) {
        read_CAN_msg(Buffer::RX0);
        modify_register(Register::InterruptFlag, InterruptFlag::RX0, 0);
        return MessageState::MessageFetched;
    } else if (status & Status::RX1InterruptFired) {
        read_CAN_msg(Buffer::RX1);
        modify_register(Register::InterruptFlag, InterruptFlag::RX1, 0);
        return MessageState::MessageFetched;
    }
    return MessageState::NoMessage;
}
