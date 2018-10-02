#ifndef __MCP2515_H__
#define __MCP2515_H__

#include <MCP2515Base.h>
#include <MCP2515Const.h>

namespace wlp {

    class MCP2515 {
    public:
        explicit MCP2515(MCP2515Base *base);

        uint8_t begin(uint8_t canSpeed, uint8_t clockSpeed);
        uint8_t set_filter(uint8_t num, uint32_t data);
        uint8_t set_mask(uint8_t num, uint32_t data);
        uint8_t send_buffer(uint32_t id, uint8_t len, uint8_t *buf);
        void read_buffer(uint8_t len, uint8_t *buf);
        uint8_t get_error();
        uint8_t get_message_status();
        uint32_t get_id();

    private:
        MCP2515Base *m_base;

        uint32_t m_id;

        uint8_t m_extFlag;
        uint8_t m_remoteRequestFlag;
        uint8_t m_filterFlag;

        uint8_t m_dataLength;
        uint8_t m_messageData[Limit::MessageBufferLength];
    };

}

#endif
