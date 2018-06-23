#ifndef _MCP2515DFS_H_
#define _MCP2515DFS_H_

#include "Cosa/SPI.hh"
#include "Cosa/OutputPin.hh"
#include "Cosa/AnalogPin.hh"
#include "Cosa/Trace.hh"
#include "Cosa/UART.hh"

#define DEBUG_MODE 1

namespace wlp {

namespace Bits {
    enum {
        SIDH = 0,  // Standard ID high
        SIDL = 1,  // Standard ID low
        EIDH = 2,  // Extended ID high
        EIDL = 3  // Extended ID low
    };
}

namespace Mask {
    enum {
        ExtendedID = 0x08,
        DLC = 0x0F,
        RemoteRequest = 0x40,
    };
}

namespace RXControlMask {
    enum {
        AcceptAny = 0x60,
        AcceptOnlyExtendedID = 0x40,
        AcceptOnlyStandardID = 0x20,
        AcceptBUKT = 0x04,
        AcceptAnyID = 0x00,
    };
}

namespace TXControlMask {
    enum {
        RequestInProcess = 0x08,
        Error = 0x10
    };
}

namespace Status {
    enum {
        RX0InterruptFired = 0x01,
        RX1InterruptFired = 0x02,
        TX0InterruptFired = 0x08,
        TX1InterruptFired = 0x20,
        TX2InterruptFired = 0x80,
        TX0Pending = 0x04,
        TX1Pending = 0x10,
        TX2Pending = 0x40,
    };
}

namespace StatusMask {
    enum {
        TXPendingMask = 0x54,
        TXInterruptMask = 0xA8,
        RXInterruptMask = 0x03,
    };
}

namespace Error {
    enum {
        None = 0x0,
        ControlError = 0x01,
    };
}

namespace ErrorMask {
    enum {
        Any = 0b11111000
    };
}

namespace Register {
    enum {
        // RXF#@@@@ are filter registers, 0 through 5
        // SID: standard identifier bits (16)
        // EID: extended identifier bits (16)
        RXF0SIDH = 0x00,
        RXF0SIDL = 0x01,
        RXF0EIDH = 0x02,
        RXF0EIDL = 0x03,
        RXF1SIDH = 0x04,
        RXF1SIDL = 0x05,
        RXF1EIDH = 0x06,
        RXF1EIDL = 0x07,
        RXF2SIDH = 0x08,
        RXF2SIDL = 0x09,
        RXF2EIDH = 0x0A,
        RXF2EIDL = 0x0B,
        RXF3SIDH = 0x10,
        RXF3SIDL = 0x11,
        RXF3EIDH = 0x12,
        RXF3EIDL = 0x13,
        RXF4SIDH = 0x14,
        RXF4SIDL = 0x15,
        RXF4EIDH = 0x16,
        RXF4EIDL = 0x17,
        RXF5SIDH = 0x18,
        RXF5SIDL = 0x19,
        RXF5EIDH = 0x1A,
        RXF5EIDL = 0x1B,
        Status = 0x0E,
        Control = 0x0F,
        // RXM#@@@@ are mask registers, 0 through 5
        // SID: standard identifier bits (16)
        // EID: extended identifier bits (16)
        RXM0SIDH = 0x20,
        RXM0SIDL = 0x21,
        RXM0EIDH = 0x22,
        RXM0EIDL = 0x23,
        RXM1SIDH = 0x24,
        RXM1SIDL = 0x25,
        RXM1EIDH = 0x26,
        RXM1EIDL = 0x27,
        RateConfig3 = 0x28,
        RateConfig2 = 0x29,
        RateConfig1 = 0x2A,
        InterruptEnable = 0x2B,
        InterruptFlag = 0x2C,
        ErrorFlag = 0x2D,
        // Transmit/receive buffers
        // 16 bytes each: [CTRL][<------ 15 bytes ------>]
        // SIDH is the first content byte.
        TXB0CTRL = 0x30,
        TXB0SIDH = 0x31,
        TXB1CTRL = 0x40,
        TXB1SIDH = 0x41,
        TXB2CTRL = 0x50,
        TXB2SIDH = 0x51,
        RXB0CTRL = 0x60,
        RXB0SIDH = 0x61,
        RXB1CTRL = 0x70,
        RXB1SIDH = 0x71,
    };
}

namespace Buffer {
    enum {
        TX0 = Register::TXB0SIDH,
        TX1 = Register::TXB1SIDH,
        TX2 = Register::TXB2SIDH,
        RX0 = Register::RXB0SIDH,
        RX1 = Register::RXB1SIDH,
    };
}

namespace InterruptMask {
    enum {
        TXAll = 0x1C,
        TX01 = 0x0C,    // Only buffers 0 and 1
        RXAll = 0x03,
        None = 0x00
    };
}

namespace Instruction {
    enum {
        Fetch = 0x00,
        Write = 0x02,
        Read = 0x03,
        Modify = 0x05,
        LoadTX0 = 0x40,
        LoadTX1 = 0x42,
        LoadTX2 = 0x44,
        ReadRX0 = 0x90,
        ReadRX1 = 0x94,
        ReadStatus = 0xA0,
        Reset = 0xC0
    };
}

namespace ControlMask {
    enum {
        Mode = 0xE0,
    };
}

namespace Mode {
    enum {
        Normal = 0x00,
        OneShot = 0x08,
        AbortTX = 0x10,
        Sleep = 0x20,
        Loopback = 0x40,
        ListenOnly = 0x60,
        Config = 0x80,
        Powerup = 0xE0,
    };
}

namespace InterruptFlag {
    enum {
        RX0 = 0x01,
        RX1 = 0x02,
        TX0 = 0x04,
        TX1 = 0x08,
        TX2 = 0x10,
        Error = 0x20,
        Wakeup = 0x40,
    };
}

#define MCP_16MHz_1000kBPS_CFG1 (0x00)
#define MCP_16MHz_1000kBPS_CFG2 (0xD0)
#define MCP_16MHz_1000kBPS_CFG3 (0x82)

#define MCP_16MHz_500kBPS_CFG1 (0x00)
#define MCP_16MHz_500kBPS_CFG2 (0xF0)
#define MCP_16MHz_500kBPS_CFG3 (0x86)

#define MCP_16MHz_250kBPS_CFG1 (0x41)
#define MCP_16MHz_250kBPS_CFG2 (0xF1)
#define MCP_16MHz_250kBPS_CFG3 (0x85)

#define MCP_16MHz_200kBPS_CFG1 (0x01)
#define MCP_16MHz_200kBPS_CFG2 (0xFA)
#define MCP_16MHz_200kBPS_CFG3 (0x87)

#define MCP_16MHz_125kBPS_CFG1 (0x03)
#define MCP_16MHz_125kBPS_CFG2 (0xF0)
#define MCP_16MHz_125kBPS_CFG3 (0x86)

#define MCP_16MHz_100kBPS_CFG1 (0x03)
#define MCP_16MHz_100kBPS_CFG2 (0xFA)
#define MCP_16MHz_100kBPS_CFG3 (0x87)

#define MCP_16MHz_95kBPS_CFG1 (0x03)
#define MCP_16MHz_95kBPS_CFG2 (0xAD)
#define MCP_16MHz_95kBPS_CFG3 (0x07)

#define MCP_16MHz_83k3BPS_CFG1 (0x03)
#define MCP_16MHz_83k3BPS_CFG2 (0xBE)
#define MCP_16MHz_83k3BPS_CFG3 (0x07)

#define MCP_16MHz_80kBPS_CFG1 (0x03)
#define MCP_16MHz_80kBPS_CFG2 (0xFF)
#define MCP_16MHz_80kBPS_CFG3 (0x87)

#define MCP_16MHz_50kBPS_CFG1 (0x07)
#define MCP_16MHz_50kBPS_CFG2 (0xFA)
#define MCP_16MHz_50kBPS_CFG3 (0x87)

#define MCP_16MHz_40kBPS_CFG1 (0x07)
#define MCP_16MHz_40kBPS_CFG2 (0xFF)
#define MCP_16MHz_40kBPS_CFG3 (0x87)

#define MCP_16MHz_33kBPS_CFG1 (0x09)
#define MCP_16MHz_33kBPS_CFG2 (0xBE)
#define MCP_16MHz_33kBPS_CFG3 (0x07)

#define MCP_16MHz_31k25BPS_CFG1 (0x0F)
#define MCP_16MHz_31k25BPS_CFG2 (0xF1)
#define MCP_16MHz_31k25BPS_CFG3 (0x85)

#define MCP_16MHz_25kBPS_CFG1 (0X0F)
#define MCP_16MHz_25kBPS_CFG2 (0XBA)
#define MCP_16MHz_25kBPS_CFG3 (0X07)

#define MCP_16MHz_20kBPS_CFG1 (0x0F)
#define MCP_16MHz_20kBPS_CFG2 (0xFF)
#define MCP_16MHz_20kBPS_CFG3 (0x87)

#define MCP_16MHz_10kBPS_CFG1 (0x1F)
#define MCP_16MHz_10kBPS_CFG2 (0xFF)
#define MCP_16MHz_10kBPS_CFG3 (0x87)

#define MCP_16MHz_5kBPS_CFG1 (0x3F)
#define MCP_16MHz_5kBPS_CFG2 (0xFF)
#define MCP_16MHz_5kBPS_CFG3 (0x87)

#define MCP_16MHz_666kBPS_CFG1 (0x00)
#define MCP_16MHz_666kBPS_CFG2 (0xA0)
#define MCP_16MHz_666kBPS_CFG3 (0x04)

#define CAN_5KBPS           1
#define CAN_10KBPS          2
#define CAN_20KBPS          3
#define CAN_25KBPS          4
#define CAN_31K25BPS        5
#define CAN_33KBPS          6
#define CAN_40KBPS          7
#define CAN_50KBPS          8
#define CAN_80KBPS          9
#define CAN_83K3BPS         10
#define CAN_95KBPS          11
#define CAN_100KBPS         12
#define CAN_125KBPS         13
#define CAN_200KBPS         14
#define CAN_250KBPS         15
#define CAN_500KBPS         16
#define CAN_666KBPS         17
#define CAN_1000KBPS        18

namespace MessageState {
    enum {
        MessagePending = 0x00,
        MessageFetched = 0x01,
        NoMessage = 0x02,
    };
}

namespace Result {
    enum {
        OK = 0x00,
        Failed = 0x01,
        AwaitBufferTimedOut = 0x02,
        SendTimedOut = 0x03,
        AllBuffersBusy = 0x04,
    };
}

namespace Limit {
    enum {
        AwaitBufferTimeout = 0x32,
        MessageBufferLength = 0x08,
        TXBufferLength = 0x10,
        TXBuffers = 0x03,
    };
}

}

#endif
