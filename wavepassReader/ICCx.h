#ifndef iccx_h
#define iccx_h
#include "ACIO.h"

#define EJECT_DELAY 1000

enum iccx_cmd {
    AC_IO_CMD_ICCx_QUEUE_LOOP_START = 0x0130,
    AC_IO_CMD_ICCx_ENGAGE = 0x0131,
    AC_IO_CMD_ICCx_POLL = 0x0134,
    AC_IO_CMD_ICCx_SET_SLOT_STATE = 0x135,
    AC_IO_CMD_ICCx_BEGIN_KEYPAD = 0x013A,
    AC_IO_CMD_ICCx_KEY_EXCHANGE = 0x0160,
    AC_IO_CMD_ICCx_FEL_ENGAGE = 0x0161,
    AC_IO_CMD_ICCx_FEL_POLL = 0x0164,
};

typedef enum ac_io_icca_slot_state {
    AC_IO_ICCA_SLOT_STATE_OPEN = 0x11,
    AC_IO_ICCA_SLOT_STATE_EJECT = 0x12,
    AC_IO_ICCA_SLOT_STATE_CLOSE = 0x00,
} icca_slot_state_t;

enum icca_sensor_state {
    /* Card eject event fired once after slot state is set to eject the card */
    AC_IO_ICCA_SENSOR_STATE_CARD_EJECTED = 0x50,
    AC_IO_ICCA_SENSOR_CARD = (1 << 1),
    AC_IO_ICCA_SENSOR_NO_CARD = (1 << 2),
    AC_IO_ICCA_SENSOR_MASK_FRONT_ON = (1 << 4),
    AC_IO_ICCA_SENSOR_MASK_BACK_ON = (1 << 5)
};

enum iccx_sensor_state {
    AC_IO_ICCx_SENSOR_CARD = 0x02,
    AC_IO_ICCx_SENSOR_NO_CARD = 0x04
};

enum iccx_card_type {
    AC_IO_ICCx_CARD_TYPE_ISO15696 = 0x00,
    AC_IO_ICCx_CARD_TYPE_FELICA = 0x01,
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
    uint8_t sensor_state;
    uint8_t card_type;
    uint8_t uid[8];
    uint8_t unk2;
    uint8_t keypad_started;
    uint8_t key_events[2];    /* circular buffer of last 2 key events, low nibble is key number (from 1 to C going from bottom to top and left to right), high nibble is an auto-increment number */
    uint16_t key_state;       /* bitfield with the currently pressed keys */
} iccx_state_t;

/* this struct is only used for slotted readers (to set locking mech etc) */
typedef struct  icca_state_s {
    uint8_t status_code;
    uint8_t sensor_state;
    uint8_t uid[8];
    uint8_t card_type;
    uint8_t keypad_started;
    uint8_t key_events[2];
    uint16_t key_state;
} icca_state_t;

typedef struct iccx_key_state_s {
    uint8_t key1;
    uint8_t key2;
    uint8_t key3;
    uint8_t key4;
} iccx_key_state_t;

bool iccx_init(uint8_t node_id, bool encrypted);
bool iccx_scan_card(uint8_t *type, uint8_t *uid, uint16_t *key_state, bool encrypted);
bool iccx_eject_card(icca_slot_state_t post_state);

#endif
