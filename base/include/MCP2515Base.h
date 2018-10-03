#ifndef __BASE_MCP2515_H__
#define __BASE_MCP2515_H__

#include <stdint.h>

namespace wlp {

    namespace Instruction {
        enum {
            Fetch      = 0x00,
            Write      = 0x02,
            Read       = 0x03,
            Modify     = 0x05,
            ReadStatus = 0xA0,
            Reset      = 0xC0
        };
    }

    class MCP2515Base {
    public:
        virtual void reset(void) = 0;
        virtual uint8_t read_status(void) = 0;
        virtual uint8_t read_register(uint8_t address) = 0;
        virtual void read_registers(uint8_t address, uint8_t values[], uint8_t n) = 0;
        virtual void set_register(uint8_t address, uint8_t value) = 0;
        virtual void set_registers(uint8_t address, uint8_t values[], uint8_t n) = 0;
        virtual void modify_register(uint8_t address, uint8_t mask, uint8_t data) = 0;
    };

}

#endif
