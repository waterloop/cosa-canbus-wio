#ifndef __MCP2515_CONST_H__
#define __MCP2515_CONST_H__

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
            LoadTX0 = 0x40,
            LoadTX1 = 0x42,
            LoadTX2 = 0x44,
            ReadRX0 = 0x90,
            ReadRX1 = 0x94,
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

    /* Clock Speeds */
    enum {
        MCP_16MHz = 1,
        MCP_8MHz  = 2
    };

    enum {
        CAN_1000KBPS = 0,
        CAN_500KBPS,
        CAN_250KBPS,
        CAN_200KBPS,
        CAN_125KBPS,
        CAN_100KBPS,
        CAN_50KBPS,
        CAN_80KBPS,
        CAN_40KBPS,
        CAN_31K25BPS,
        CAN_20KBPS,
        CAN_10KBPS,
        CAN_5KBPS,

        CAN_666KBPS,
        CAN_95KBPS,
        CAN_83K3BPS,
        CAN_33KBPS,
        CAN_25KBPS,
    };

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
