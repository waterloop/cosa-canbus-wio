#ifndef MCP2515_H
#define MCP2515_H

#include <Cosa/OutputPin.hh>
#include "MCP2515_const.h"

namespace wlp {

    class MCP2515: public SPI::Driver {
    public:
        MCP2515(Board::DigitalPin cs_pin = Board::D10);
        /*
        ** Initialize the shield and set the transmission rate to `can_speed`.
        **
        ** - this must be called before any other method is called,
        **   else the latter will fail silently
        ** - `can_speed` must be one of the constants of the form CAN_[NUM]KBPS
        **   defined in MCP2515_const.h
        **
        ** Return:
        **   Result::OK on success
        **   Result::Failed on failure
        */
        uint8_t begin(uint8_t can_speed);
        /*
        ** Set filter number `num` to  `data`. Filters 0 through 5 are available.
        **
        ** This will result in only messages with `id` bitwise matching
        ** `data` for at least one of the filters to be processed.
        **
        ** Return values match those of `begin`.
        */
        uint8_t set_filter(uint8_t num, uint32_t data);
        /*
        ** Set mask number `num` to `data`. Masks 0 and 1 are available.
        **
        ** This will result in only the bits of `id` set to 1 in one of the masks
        ** to be checked against the filters.
        **
        ** Return values match those of `begin`.
        **
        ** Examples follow; all masks and filters not given are assumed to be 0:
        **
        **   MASK_0 = 0x1FFFFFFF
        **   FILTER_0 = 0x00001567
        **     will match bits 0-28 of `id` against 0x00001567, i.e.
        **     only messages with `id` == 0x00001567 will be processed
        **
        **   MASK_0 = 0x1FFFFFF0
        **   MASK_1 = 0x1FFFFF00
        **   FILTER_0 = 0x00001560
        **     will match bits 4-28 of `id` against 0x00001560, i.e. messages
        **     with `id` in the range 0x00001560...0x0000156F will be processed.
        **     Note that MASK_1 is redundant here: MASK_0 | MASK_1 == MASK_0
        **
        **   FILTER_0 = 0x00001567
        **   FILTER_1 = 0x00001569
        **     this _would_ match only messages with `id` == 0x00001567
        **     or `id` == 0x00001569, had MASK_0 or MASK_1 been set as needed.
        **     With MASK_0 == 0x00000000 == MASK_1, this matches every message.
        */
        uint8_t set_mask(uint8_t num, uint32_t data);
        /*
        ** Send buffer `buf` of length `len` in a message with id `id`.
        **
        ** - `len` shall be no greater than Limit::MessageBufferLength.
        ** - `id` may be standard (only the lower 11 bits set)
        **    or extended (29 lower bits are set).
        **
        **    Note: if a standard `id` is used, ensure that the extended
        **    (upper 21) bits of all masks on the receiving side are set to zero.
        **
        ** Return:
        **   Result::AwaitBufferTimedOut if waiting for a free buffer timed out
        **   Result::SendTimedOut if sending the message timed out
        **   Result::OK if the message was sent successfully
        */
        uint8_t send_buffer(uint32_t id, uint8_t len, uint8_t *buf);
        /*
        ** Read a received message (there may be more than one at a time!)
        ** and store its contents in `buf`, up to length `len`.
        **
        ** Return:
        **   MessageState::MessageFetched on success
        **   MessageState::NoMessage on failure
        */
        void read_buffer(uint8_t len, uint8_t *buf);
        /*
        ** Get the current error status. Error::None indicates no error.
        **
        ** FIXME: Right now, every error is reported as Error::ControlError.
        */
        uint8_t get_error();
        /*
        ** Check if a message is received an pending read.
        **
        ** Return:
        **   MessageState::MessagePending if (at least one) message is pending
        **   MessageState::NoMessage otherwise
        */
        uint8_t get_message_state();
        /*
        ** Get the id of the last successfully read message.
        **
        ** The return value is unspecified if no message has been read yet.
        */
        uint32_t get_id();

    private:
        uint32_t  id;  // ID of this device on CAN bus
        // Flags
        uint8_t   ext_flag;  // is ID extended (29 LSB) or not (11 LSB)?
        uint8_t   remote_request_flag;
        uint8_t   filter_flag;
        // Pending message data
        uint8_t   data_length;
        uint8_t   message_data[Limit::MessageBufferLength];

        // Control commands
        uint8_t configure_rate(const uint8_t bus_rate);
        void init_buffers();
        uint8_t read_status();
        void reset();
        uint8_t set_control_mode(const uint8_t new_mode);

        // Register commands
        uint8_t read_register(const uint8_t address);
        void read_registers(
                const uint8_t address,
                uint8_t values[], 
                const uint8_t n);
        void modify_register(
                const uint8_t address,
                const uint8_t mask,
                const uint8_t data);
        void set_register(
                const uint8_t address,
                const uint8_t value);
        void set_registers(
                const uint8_t address,
                uint8_t values[],
                const uint8_t n);

        // Generic transmission commands
        void write_id(
                const uint8_t mcp_addr,
                const uint32_t id);
        uint32_t read_id(const uint8_t mcp_addr);
        void write_CAN_msg(const uint8_t buffer_sidh_addr);
        void read_CAN_msg(const uint8_t buffer_sidh_addr);
        void start_transmit(const uint8_t mcp_addr);
        uint8_t get_next_free_buf(uint8_t *txbuf);

        // Message operations
        void set_msg(uint32_t id, uint8_t len, uint8_t *data);
        void clear_msg();
        uint8_t read_msg();
        uint8_t send_msg();
    };

}

#endif  // MCP2515_H
