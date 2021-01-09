#ifndef acio_h
#define acio_h

#include <Arduino.h>

#define ac_io_u16(x) __builtin_bswap16(x)
#define ac_io_u32(x) __builtin_bswap32(x)

#define AC_IO_SOF 0xAA
#define AC_IO_ESCAPE 0xFF
enum ac_io_cmd {
    AC_IO_CMD_ASSIGN_ADDRS = 0x0001,
    AC_IO_CMD_GET_VERSION = 0x0002,
    AC_IO_CMD_START_UP = 0x0003,
    AC_IO_CMD_KEEPALIVE = 0x0080,
    /* Yet unknown command encountered first on jubeat (1) */
    AC_IO_CMD_UNKN_00FF = 0x00FF,
    AC_IO_CMD_CLEAR = 0x0100,
};

struct ac_io_version {
    /* Names taken from some debug text in libacio.dll */
    uint32_t type;
    uint8_t flag;
    uint8_t major;
    uint8_t minor;
    uint8_t revision;
    char product_code[4];
    char date[16];
    char time[16];
};

struct ac_io_message {
    uint8_t addr; /* High bit: clear = req, set = resp */

    union {
        struct {
            uint16_t code;
            uint8_t seq_no;
            uint8_t nbytes;

            union {
                uint8_t raw[0xFF];
                uint8_t count;
                uint8_t status;
                struct ac_io_version version;
            };
        } cmd;

        struct {
            uint8_t nbytes;
            uint8_t raw[0xFF]; /* 0xFFucked if I know */
        } bcast;
    };
};

int acio_get_counter_and_increase();
bool acio_send(const uint8_t *buffer, int length);
int acio_receive(uint8_t *buffer, int size);
bool acio_send_and_recv(struct ac_io_message *msg, int resp_size);
bool acio_open();

#endif
