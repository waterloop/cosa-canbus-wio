#ifndef __LINUX_MCP2515_H__
#define __LINUX_MCP2515_H__

#include <MCP2515Base.h>
#include <linux/spi/spidev.h>

namespace wlp {
    namespace linux {
        class MCP2515 : public wlp::MCP2515Base {
        public:
            MCP2515(const char *dev, int busSpeed);

            int begin(void);

            void reset(void) override;
            uint8_t read_status(void) override;
            uint8_t read_register(uint8_t address) override;
            void read_registers(uint8_t address, uint8_t values[], uint8_t n) override;
            void set_register(uint8_t address, uint8_t value) override;
            void set_registers(uint8_t address, uint8_t values[], uint8_t n) override;
            void modify_register(uint8_t address, uint8_t mask, uint8_t data) override;

        private:
            const char *m_dev;
            uint8_t m_speed;
            uint8_t m_bitsPerWord;
            uint8_t m_mode;
            uint8_t m_lsbFirst;
            int m_fd;

            spi_ioc_transfer m_spiBuffer[2];
        };
    }
}

#endif
