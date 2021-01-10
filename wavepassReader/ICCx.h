#ifndef iccx_h
#define iccx_h
#include "ACIO.h"

enum iccx_cmd {
    AC_IO_CMD_ICCx_QUEUE_LOOP_START = 0x0130,
    AC_IO_CMD_ICCx_ENGAGE = 0x0131,
    AC_IO_CMD_ICCx_POLL = 0x0134,
    AC_IO_CMD_ICCx_BEGIN_KEYPAD = 0x013A,
    AC_IO_CMD_ICCx_KEY_EXCHANGE = 0x0160,
    AC_IO_CMD_ICCx_FEL_ENGAGE = 0x0161,
    AC_IO_CMD_ICCx_FEL_POLL = 0x0164,
};

enum iccx_sensor_state {
    AC_IO_ICCx_SENSOR_CARD = 0x02,
    AC_IO_ICCx_SENSOR_NO_CARD = 0x04
};

enum iccx_card_type {
    AC_IO_ICCx_CARD_TYPE_ISO15696 = 0x30,
    AC_IO_ICCx_CARD_TYPE_FELICA = 0x31,
};

enum iccx_keypad_mask {
    ICCx_KEYPAD_MASK_EMPTY = (1 << 0),
    ICCx_KEYPAD_MASK_3 = (1 << 1),
    ICCx_KEYPAD_MASK_6 = (1 << 2),
    ICCx_KEYPAD_MASK_9 = (1 << 3),

    ICCx_KEYPAD_MASK_0 = (1 << 8),
    ICCx_KEYPAD_MASK_1 = (1 << 9),
    ICCx_KEYPAD_MASK_4 = (1 << 10),
    ICCx_KEYPAD_MASK_7 = (1 << 11),

    ICCx_KEYPAD_MASK_00 = (1 << 12),
    ICCx_KEYPAD_MASK_2 = (1 << 13),
    ICCx_KEYPAD_MASK_5 = (1 << 14),
    ICCx_KEYPAD_MASK_8 = (1 << 15),
};

typedef struct  iccx_state_s {
/*  //old format (unencrypted)
    uint8_t status_code;
    uint8_t sensor_state;
    uint8_t uid[8];
    uint8_t card_type;
    uint8_t keypad_started;
    uint8_t key_events[2];
    uint16_t key_state;
    */
    uint8_t sensor_state;
    uint8_t card_type;
    uint8_t uid[8];
    uint8_t unk2;
    uint8_t unk3;
    uint8_t unk4[4];
} iccx_state_t;

typedef struct iccx_key_state_s {
    uint8_t key1;
    uint8_t key2;
    uint8_t key3;
    uint8_t key4;
} iccx_key_state_t;

bool iccx_init(uint8_t node_id);
bool iccx_scan_card(uint8_t *type, uint8_t *uid);

#endif
