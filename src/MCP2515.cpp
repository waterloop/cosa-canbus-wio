#include "MCP2515.h"

MCP2515::MCP2515(Board::DigitalPin cs_pin)
    : cs_pin(cs_pin), spi_device(cs_pin) {}

// PUBLIC

uint8_t MCP2515::begin(const uint8_t bus_rate) {
    reset();

    uint8_t res = set_control_mode(MODE_CONFIG);
    if (res != MCP2515_OK) {
        #if DEBUG_MODE
        trace << "Switching to config mode failed\n";
        #endif
        return CAN_FAILINIT;
    }
    #if DEBUG_MODE
        trace << "Config mode entered successfully\n";
    #endif

    if (configure_rate(bus_rate) != MCP2515_OK) {
        #if DEBUG_MODE
        trace << "Setting transmission rate failed\n";
        #endif
        return CAN_FAILINIT;
    }
    #if DEBUG_MODE
    trace << "Transmission rate set successfully\n";
    #endif

    init_buffers();
    // Enable interrupts on receipt
    set_register(MCP_CANINTE, MCP_RX0IF | MCP_RX1IF);

    #if (DEBUG_RXANY==1)
    // Receive messages from any source
    modify_register(MCP_RXB0CTRL, MCP_RXB_RX_MASK | MCP_RXB_BUKT_MASK,
                    MCP_RXB_RX_ANY | MCP_RXB_BUKT_MASK);
    modify_register(MCP_RXB1CTRL, MCP_RXB_RX_MASK, MCP_RXB_RX_ANY);
    #else
    // Receive messages from standard an extended ids only
    modify_register(MCP_RXB0CTRL, MCP_RXB_RX_MASK | MCP_RXB_BUKT_MASK,
                    MCP_RXB_RX_STDEXT | MCP_RXB_BUKT_MASK );
    modify_register(MCP_RXB1CTRL, MCP_RXB_RX_MASK, MCP_RXB_RX_STDEXT);
    #endif

    res = set_control_mode(MODE_NORMAL);
    if (res) {
        #if DEBUG_MODE
        trace << "Retuning to normal mode failed\n";
        #endif
        return CAN_FAILINIT;
    }

    #if DEBUG_MODE
    trace << "Returned to normal mode\n";
    #endif
    return CAN_OK;
}

uint8_t MCP2515::init_mask(uint8_t num, uint8_t ext, uint32_t data) {

    uint8_t res = MCP2515_OK;
    #if DEBUG_MODE
    trace << "Starting mask setup\n";
    #endif
    res = set_control_mode(MODE_CONFIG);
    if (res > 0) {
        #if DEBUG_MODE
        trace << "Switching to config mode failed\n";
        #endif
        return CAN_FAIL;
    }

    if (num == 0)
        write_id(MCP_RXM0SIDH, ext, data);
    else if(num == 1)
        write_id(MCP_RXM1SIDH, ext, data);
    else
        return MCP2515_FAIL;

    res = set_control_mode(MODE_NORMAL);
    if (res > 0) {
        #if DEBUG_MODE
        trace << "Returning to normal model failed\nMask setup failed\n";
        #endif
        return res;
    }
    #if DEBUG_MODE
    trace << "Mask setup successfull\n";
    #endif

    return res;
}

uint8_t MCP2515::init_filter(uint8_t num, uint8_t ext, uint32_t data)
{
    uint8_t res = MCP2515_OK;
    #if DEBUG_MODE
    trace << "Starting filter setup\n";
    #endif
    res = set_control_mode(MODE_CONFIG);
    if(res > 0) {
        #if DEBUG_MODE
        trace << "Switching to config mode failed\n";
        #endif
        return res;
    }

    switch(num) {
        case 0:
            write_id(MCP_RXF0SIDH, ext, data);
        break;

        case 1:
            write_id(MCP_RXF1SIDH, ext, data);
        break;

        case 2:
            write_id(MCP_RXF2SIDH, ext, data);
        break;

        case 3:
            write_id(MCP_RXF3SIDH, ext, data);
        break;

        case 4:
            write_id(MCP_RXF4SIDH, ext, data);
        break;

        case 5:
            write_id(MCP_RXF5SIDH, ext, data);
        break;

        default:
        res = MCP2515_FAIL;
    }

    res = set_control_mode(MODE_NORMAL);
    if(res > 0) {
        #if DEBUG_MODE
        trace << "Returning to normal mode failed\nFilter setup failed\n";
        #endif
        return res;
    }

    #if DEBUG_MODE
    trace << "Filter setup successful\n";
    #endif

    return res;
}

uint8_t MCP2515::send_buffer(uint32_t id, uint8_t ext, uint8_t len, uint8_t *buf)
{
    set_msg(id, ext, len, buf);
    return send_msg();
}

uint8_t MCP2515::read_buffer(uint8_t *buf)
{
    this->read_msg();
    for (int i = 0; i < this->data_length; ++i)
        buf[i] = this->message_data[i];
    return this->data_length;
}

bool MCP2515::get_message_received_state(void) {
    uint8_t res = read_status();
    return (res & MCP_STAT_RXIF_MASK) ? CAN_MSGAVAIL : CAN_NOMSG;
}

uint8_t MCP2515::get_error(void) {
    uint8_t eflg = read_register(MCP_EFLG);
    return (eflg & MCP_EFLG_ERRORMASK) ? CAN_CTRLERROR : CAN_OK;
}

uint32_t MCP2515::get_id(void) {
    return this->id;
}

// PRIVATE

void MCP2515::reset(void)
{
    cs_pin.low();
    spi.begin();
    spi.acquire(&spi_device);
    spi.transfer(MCP_RESET);
    spi.release();
    spi.end();
    cs_pin.high();
}

uint8_t MCP2515::read_register(const uint8_t address)
{
    cs_pin.low();
    spi.begin();
    spi.acquire(&spi_device);
    spi.transfer(MCP_READ);
    spi.transfer(address);
    uint8_t ret = spi.transfer(0x00);
    spi.release();
    spi.end();
    cs_pin.high();
    return ret;
}

void MCP2515::read_registers(const uint8_t address, uint8_t values[], const uint8_t n)
{
	cs_pin.low();
    spi.begin();
    spi.acquire(&spi_device);
    spi.transfer(MCP_READ);
    spi.transfer(address);
    for (uint8_t i = 0; i < n; i++) {
        // mcp2515 will auto-increment address pointer
        values[i] = spi.transfer(0x00);
    }
    spi.release();
    spi.end();
	cs_pin.high();
}

void MCP2515::set_register(const uint8_t address, const uint8_t value)
{
    cs_pin.low();
    spi.begin();
    spi.acquire(&spi_device);
    spi.transfer(MCP_WRITE);
    spi.transfer(address);
    spi.transfer(value);
    spi.release();
    spi.end();
    cs_pin.high();
}

void MCP2515::set_registers(const uint8_t address, uint8_t values[], const uint8_t n)
{
    cs_pin.low();
    spi.begin();
    spi.acquire(&spi_device);
    spi.transfer(MCP_WRITE);
    spi.transfer(address);
    for (uint8_t i = 0; i < n; i++) {
        // mcp2515 will auto-increment address pointer
        values[i] = spi.transfer(0x00);
    }
    spi.release();
    spi.end();
    cs_pin.high();
}

void MCP2515::modify_register(const uint8_t address, const uint8_t mask, const uint8_t data)
{
    cs_pin.low();
    spi.begin();
    spi.acquire(&spi_device);
    spi.transfer(MCP_BITMOD);
    spi.transfer(address);
    spi.transfer(mask);
    spi.transfer(data);
    spi.release();
    spi.end();
    cs_pin.high();
}

uint8_t MCP2515::read_status(void)
{
	cs_pin.low();
    spi.begin();
    spi.acquire(&spi_device);
    spi.transfer(MCP_READ_STATUS);
    uint8_t i = spi.transfer(0x00);
    spi.release();
    spi.end();
	cs_pin.high();
	return i;
}

uint8_t MCP2515::set_control_mode(const uint8_t new_mode)
{
    modify_register(MCP_CANCTRL, MODE_MASK, new_mode);
    uint8_t i = read_register(MCP_CANCTRL) & MODE_MASK;
    return (i == new_mode) ? MCP2515_OK : MCP2515_FAIL;
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
        set_register(MCP_CNF1, cfg1);
        set_register(MCP_CNF2, cfg2);
        set_register(MCP_CNF3, cfg3);
        return MCP2515_OK;
    } else {
        return MCP2515_FAIL;
    }
}

void MCP2515::init_buffers(void)
{
    write_id(MCP_RXM0SIDH, 0, 0);
    write_id(MCP_RXM1SIDH, 0, 0);
    write_id(MCP_RXF0SIDH, 0, 0);
    write_id(MCP_RXF1SIDH, 0, 0);
    write_id(MCP_RXF2SIDH, 0, 0);
    write_id(MCP_RXF3SIDH, 0, 0);
    write_id(MCP_RXF4SIDH, 0, 0);
    write_id(MCP_RXF5SIDH, 0, 0);

    // Clear the three transmit buffers
    uint8_t a1 = MCP_TXB0CTRL;
    uint8_t a2 = MCP_TXB1CTRL;
    uint8_t a3 = MCP_TXB2CTRL;
    for (uint8_t i = 0; i < 14; i++) {
        set_register(a1, 0);
        set_register(a2, 0);
        set_register(a3, 0);
        a1++;
        a2++;
        a3++;
    }
    set_register(MCP_RXB0CTRL, 0);
    set_register(MCP_RXB1CTRL, 0);
}

void MCP2515::write_id(const uint8_t mcp_addr, const uint8_t is_extended,
                       const uint32_t id)
{
    uint16_t canid = (uint16_t)(id & 0x0FFFF);
    uint8_t tbufdata[4];

    if (is_extended) {
        tbufdata[MCP_EID0] = (uint8_t) (canid & 0xFF);
        tbufdata[MCP_EID8] = (uint8_t) (canid / 256);
        canid = (uint16_t)( id / 0x10000L );
        tbufdata[MCP_SIDL] = (uint8_t) (canid & 0x03);
        tbufdata[MCP_SIDL] += (uint8_t) ((canid & 0x1C )*8);
        tbufdata[MCP_SIDL] |= MCP_TXB_EXIDE_M;
        tbufdata[MCP_SIDH] = (uint8_t) (canid / 32 );
    } else {
        tbufdata[MCP_SIDH] = (uint8_t) (canid / 8 );
        tbufdata[MCP_SIDL] = (uint8_t) ((canid & 0x07 )<<5);
        tbufdata[MCP_EID0] = 0;
        tbufdata[MCP_EID8] = 0;
    }
    set_registers(mcp_addr, tbufdata, 4);
}

void MCP2515::read_id(const uint8_t mcp_addr, uint8_t* is_extended, uint32_t* id)
{
    uint8_t tbufdata[4];

    *is_extended = 0;
    *id = 0;

    read_registers(mcp_addr, tbufdata, 4);

    *id = (tbufdata[MCP_SIDH]<<3) + (tbufdata[MCP_SIDL]>>5);

    if ((tbufdata[MCP_SIDL] & MCP_TXB_EXIDE_M) ==  MCP_TXB_EXIDE_M) {
        *id = (*id<<2) + (tbufdata[MCP_SIDL] & 0x03);
        *id <<= 16;
        *id = *id +(tbufdata[MCP_EID8]<<8) + tbufdata[MCP_EID0];
        *is_extended = 1;
    }
}

void MCP2515::write_CAN_msg( const uint8_t buffer_sidh_addr)
{
    uint8_t mcp_addr = buffer_sidh_addr;
    set_registers(mcp_addr + 5, this->message_data, this->data_length);
    if (remote_request_flag == 1) {
        this->data_length |= MCP_RTR_MASK;
    }
    set_register((mcp_addr+4), this->data_length);
    write_id(mcp_addr, this->ext_flag, id);
}

void MCP2515::read_CAN_msg(const uint8_t buffer_sidh_addr)
{
    uint8_t mcp_addr = buffer_sidh_addr, ctrl;
    read_id(mcp_addr, &this->ext_flag, &id);

    ctrl = read_register(mcp_addr - 1);
    this->data_length = read_register(mcp_addr + 4);

    if ((ctrl & 0x08)) {
        this->remote_request_flag = 1;
    } else {
        this->remote_request_flag = 0;
    }

    this->data_length &= MCP_DLC_MASK;
    read_registers(mcp_addr + 5, this->message_data, this->data_length);
}

void MCP2515::start_transmit(const uint8_t mcp_addr)
{
    modify_register(mcp_addr - 1, MCP_TXB_TXREQ_M, MCP_TXB_TXREQ_M);
}

uint8_t MCP2515::get_next_free_buf(uint8_t *txbuf_n)
{
    uint8_t res, ctrlval;
    uint8_t ctrlregs[MCP_N_TXBUFFERS] = {MCP_TXB0CTRL, MCP_TXB1CTRL, MCP_TXB2CTRL};

    *txbuf_n = 0x00;

    // Check all transaction buffers
    for (int i = 0; i < MCP_N_TXBUFFERS; i++) {
        ctrlval = read_register(ctrlregs[i]);
        if ((ctrlval & MCP_TXB_TXREQ_M) == 0) {
            *txbuf_n = ctrlregs[i]+1;
            return MCP2515_OK;
        }
    }
    return MCP_ALLTXBUSY;
}

uint8_t MCP2515::set_msg(uint32_t id, uint8_t ext, uint8_t len, uint8_t *data) {
    this->ext_flag = ext;
    this->id = id;
    this->data_length = len;
    for (int i = 0; i<CAN_MAX_CHAR_IN_MESSAGE; i++) {
        this->message_data[i] = data[i];
    }
    return MCP2515_OK;
}

uint8_t MCP2515::clear_msg() {
    for(int i = 0; i < this->data_length; i++) {
        this->message_data[i] = 0x00;
    }
    id = 0;
    this->data_length = 0;
    this->ext_flag = 0;
    this->remote_request_flag = 0;
    this->filter_flag = 0;
    return MCP2515_OK;
}

uint8_t MCP2515::send_msg()
{
    uint8_t res, txbuf_n;
    uint16_t ui_timeout = 0;

    do {
        res = get_next_free_buf(&txbuf_n);
        ui_timeout++;
    } while (res == MCP_ALLTXBUSY && (ui_timeout < TIMEOUTVALUE));

    if(ui_timeout == TIMEOUTVALUE)
        return CAN_GETTXBFTIMEOUT;

    ui_timeout = 0;
    write_CAN_msg(txbuf_n);
    start_transmit(txbuf_n);

    do {
        ui_timeout++;
        res = read_register(txbuf_n);
        res &= 0x08;
    } while(res && (ui_timeout < TIMEOUTVALUE));

    if(ui_timeout == TIMEOUTVALUE)
        return CAN_SENDMSGTIMEOUT;
    else
        return CAN_OK;
}

uint8_t MCP2515::read_msg()
{
    uint8_t stat = read_status(), res;

    if (stat & MCP_STAT_RX0IF) {  // Message is in buffer 0
        read_CAN_msg(MCP_RXBUF_0);
        modify_register(MCP_CANINTF, MCP_RX0IF, 0);
        res = CAN_OK;
    } else if (stat & MCP_STAT_RX1IF) {  // Message is in buffer 1
        read_CAN_msg(MCP_RXBUF_1);
        modify_register(MCP_CANINTF, MCP_RX1IF, 0);
        res = CAN_OK;
    } else {
        res = CAN_NOMSG;
    }
    return res;
}
