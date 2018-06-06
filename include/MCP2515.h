#ifndef MCP2515_H
#define MCP2515_H

#include <Cosa/OutputPin.hh>
#include "MCP2515_const.h"

class MCP2515 {
public:
    MCP2515(Board::DigitalPin cs_pin = Board::D10);
    uint8_t begin(uint8_t can_speed);
    uint8_t init_mask(uint8_t num, uint8_t ext, uint32_t data);
    uint8_t init_filter(uint8_t num, uint8_t ext, uint32_t data);
    uint8_t send_buffer(uint32_t id, uint8_t ext, uint8_t len, uint8_t *buf);
    uint8_t read_buffer(uint8_t *buf);
    uint8_t check_received(void);
    uint8_t error_state(void);
    uint32_t get_id(void);

private:
    uint32_t  id;  // ID of this device on CAN bus
    // Flags
    uint8_t   ext_flag;  // is ID extended (29 LSB) or not (11 LSB)?
    uint8_t   remote_request_flag;
    uint8_t   filter_flag;
    // Pending message data
    uint8_t   data_length;
    uint8_t   message_data[CAN_MAX_CHAR_IN_MESSAGE];
    // *Active-low* chip select pin. Set low to interface with shield.
    OutputPin cs_pin;
    // Shield interface.
    SPI::Driver spi_device;

    // Control commands
    uint8_t configure_rate(const uint8_t bus_rate);
    uint8_t init(const uint8_t bus_rate);
    void init_buffers(void);
    uint8_t read_status(void);
    void reset(void);
    uint8_t set_control_mode(const uint8_t new_mode);

    // Register commands
    uint8_t read_register(const uint8_t address);
    void read_registers(const uint8_t address,
                        uint8_t values[], 
                        const uint8_t n);
    void modify_register(const uint8_t address,
                         const uint8_t mask,
                         const uint8_t data);
    void set_register(const uint8_t address,
                      const uint8_t value);
    void set_registers(const uint8_t address,
                       uint8_t values[],
                       const uint8_t n);

    // Generic transmission commands
    void write_id(const uint8_t mcp_addr,
                  const uint8_t ext,
                  const uint32_t id);
    void read_id(const uint8_t mcp_addr,
                 uint8_t *ext,
                 uint32_t *id);
    void write_CAN_msg(const uint8_t buffer_sidh_addr);
    void read_CAN_msg(const uint8_t buffer_sidh_addr);
    void start_transmit(const uint8_t mcp_addr);
    uint8_t get_next_free_buf(uint8_t *txbuf);

    // Message operations
    uint8_t set_msg(uint32_t id, uint8_t ext, uint8_t len, uint8_t *data);
    uint8_t clear_msg();
    uint8_t read_msg();
    uint8_t send_msg();
};

#endif  // MCP2515_H
