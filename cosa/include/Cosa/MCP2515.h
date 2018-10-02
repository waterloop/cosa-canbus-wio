#ifndef __COSA_MCP2515_H__
#define __COSA_MCP2515_H__

#include <MCP2515Base.h>
#include <Cosa/OutputPin.hh>
#include <Cosa/SPI.hh>
#include <stdint.h>

namespace wlp {
    namespace cosa {

        class MCP2515 :
            public wlp::MCP2515Base,
            public SPI::Driver {
        public:
            MCP2515(Board::DigitalPin cs = Board::D10);

            void reset(void) override;
            uint8_t read_status(void) override;
            uint8_t read_register(uint8_t address) override;
            void read_registers(uint8_t address, uint8_t values[], uint8_t n) override;
            void set_register(uint8_t address, uint8_t value) override;
            void set_registers(uint8_t address, uint8_t values[], uint8_t n) override;
            void modify_register(uint8_t address, uint8_t mask, uint8_t data) override;
        };

    }
}

#endif
