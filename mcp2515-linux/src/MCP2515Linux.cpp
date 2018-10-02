#include <stddef.h>
#include <sys/mcp2515.h>
#include <sys/ioctl.h>
#include <fcntl.h>

using namespace wlp;

#ifndef ERROR
#define ERROR -1
#endif

#ifndef OK
#define OK 0
#endif

#if MCP2515_DEBUG_LEVEL >= 1
extern int printf(const char *msg, ...);
#define dprintf(...) printf(__VA_ARGS__)
#else
#define dprintf(...)
#endif

static void spi_process_transfers(int fd, spi_ioc_transfer *buf, uint8_t n) {
    int status = ioctl(fd, SPI_IOC_MESSAGE(n), buf);
    size_t len = buf[0].len;
    if (2 == n) {
        len += buf[1].len;
    }
    if (status != len) {
        if (status < 0) {
            dprintf("[ERROR] SPI failed transfer\n");
        } else {
            dprintf("[ERROR] SPI incomplete transfer\n");
        }
    }
}

static void spi_transfer1(
        int fd, spi_ioc_transfer *buf,
        uint8_t tx[], uint8_t rx[], uint32_t n) {
    buf[0].tx_buf = (uint64_t) tx;
    buf[0].rx_buf = (uint64_t) rx;
    buf[0].len = n;
    spi_process_transfers(fd, buf, 1);
}

static void spi_transfer2(
        int fd, spi_ioc_transfer *buf,
        uint8_t tx1[], uint8_t rx1[], uint32_t n1,
        uint8_t tx2[], uint8_t rx2[], uint32_t n2) {
    buf[0].tx_buf = (uint64_t) tx1;
    buf[0].rx_buf = (uint64_t) rx1;
    buf[0].len = n1;
    buf[1].tx_buf = (uint64_t) tx2;
    buf[1].rx_buf = (uint64_t) rx2;
    buf[1].len = n2;
    spi_process_transfers(fd, buf, 2);
}

linux::MCP2515::MCP2515(const char *dev, int busSpeed) :
        m_dev(dev),
        m_speed(busSpeed),
        m_bitsPerWord(8),
        m_mode(0),
        m_lsbFirst(0) {
    m_spiBuffer[0] = {};
    m_spiBuffer[0].speed_hz = m_speed;
    m_spiBuffer[0].bits_per_word = m_bitsPerWord;
    m_spiBuffer[1] = {};
    m_spiBuffer[1].speed_hz = m_speed;
    m_spiBuffer[1].bits_per_word = m_bitsPerWord;
}

int linux::MCP2515::begin(void) {
    m_fd = open(m_dev, O_RDWR);
    if (ERROR == m_fd) {
        dprintf("[ERROR] Failed to open device: %s\n", m_dev);
        return ERROR;
    }
    if (ioctl(m_fd, SPI_IOC_WR_MODE, &m_mode)) {
        dprintf("[ERROR] Failed to set SPI mode\n");
        return ERROR;
    }
    if (ioctl(m_fd, SPI_IOC_WR_LSB_FIRST, &m_lsbFirst)) {
        dprintf("[ERROR] Failed to set LSB First\n");
        return ERROR;
    }
    if (ioctl(m_fd, SPI_IOC_WR_BITS_PER_WORD, &m_bitsPerWord)) {
        dprintf("[ERROR] Failed to set bits per word\n");
        return ERROR;
    }
    if (ioctl(m_fd, SPI_IOC_WR_MAX_SPEED_HZ, &m_speed)) {
        dprintf("[ERROR] Failed to set SPI speed\n");
        return ERROR;
    }
    return OK;
}

void linux::MCP2515::reset(void) {
    uint8_t ins = Instruction::Reset;
    spi_transfer1(m_fd, m_spiBuffer, &ins, nullptr, 1);
}

uint8_t linux::MCP2515::read_status(void) {
    uint8_t tx[2] = {Instruction::ReadStatus, Instruction::Fetch};
    uint8_t rx[2];
    spi_transfer1(m_fd, m_spiBuffer, tx, rx, 2);
    return rx[1];
}

uint8_t linux::MCP2515::read_register(uint8_t address) {
    uint8_t tx[3] = {Instruction::Read, address, Instruction::Fetch};
    uint8_t rx[3];
    spi_transfer1(m_fd, m_spiBuffer, tx, rx, 3);
    return rx[2];
}

void linux::MCP2515::read_registers(uint8_t address, uint8_t values[], uint8_t n) {
    uint8_t tx[2] = {Instruction::Read, address};
    spi_transfer2(
        m_fd, m_spiBuffer,
        tx, nullptr, 2,
        nullptr, values, n);
}

void linux::MCP2515::set_register(uint8_t address, uint8_t value) {
    uint8_t tx[3] = {Instruction::Write, address, value};
    spi_transfer1(m_fd, m_spiBuffer, tx, nullptr, 3);
}

void linux::MCP2515::set_registers(uint8_t address, uint8_t values[], uint8_t n) {
    uint8_t tx[2] = {Instruction::Write, address};
    spi_transfer2(
        m_fd, m_spiBuffer,
        tx, nullptr, 2,
        values, nullptr, n);
}

void linux::MCP2515::modify_register(uint8_t address, uint8_t mask, uint8_t data) {
    uint8_t tx[4] = {Instruction::Modify, address, mask, data};
    spi_transfer1(m_fd, m_spiBuffer, tx, nullptr, 4);
}
