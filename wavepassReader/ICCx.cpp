#include "ICCx.h"
#include "Cipher.h"
//#define ICCX_DEBUG

static uint8_t ard_key[4] = {0x29,0x23,0xbe,0x84};
static uint8_t dev_key[4] = {0,0,0,0};
Cipher crypto;

static bool iccx_key_exchange(uint8_t node_id)
{
    struct ac_io_message msg;

    msg.addr = node_id;
    msg.cmd.code = ac_io_u16(AC_IO_CMD_ICCx_KEY_EXCHANGE);
    msg.cmd.nbytes = 4;
    memcpy(&msg.cmd.raw, ard_key, 4);

    if (!acio_send_and_recv(
            &msg, offsetof(struct ac_io_message, cmd.raw) + msg.cmd.nbytes)) {
        #ifdef ICCX_DEBUG
        Serial.println("Starting queue loop failed");
            #endif
        return false;
    }    

    memcpy(dev_key, msg.cmd.raw, 4);
    
#ifdef ICCX_DEBUG
Serial.print("Key exchange complete, got ");
for (int i=0; i<4; i++)
{
  Serial.print(" ");
      if (dev_key[i] < 0x10) Serial.print("0");
      Serial.print(dev_key[i], HEX);
}
Serial.println("");
            #endif

            unsigned long client_key = ((unsigned long) ard_key[0]) <<24 | ((unsigned long) ard_key[1]) <<16 | ((unsigned long) ard_key[2]) <<8 | (unsigned long) ard_key[3];
            unsigned long reader_key = ((unsigned long) dev_key[0]) <<24 | ((unsigned long) dev_key[1]) <<16 | ((unsigned long) dev_key[2]) <<8 | (unsigned long) dev_key[3];

            crypto.setKeys(client_key,reader_key);

    delay(200);
    return true;
}

static bool iccx_queue_loop_start(uint8_t node_id, bool encrypted)
{
    struct ac_io_message msg;

    msg.addr = node_id;
    msg.cmd.code = ac_io_u16(AC_IO_CMD_ICCx_QUEUE_LOOP_START);
    msg.cmd.nbytes = 1;
    msg.cmd.count = 0;

    if (!acio_send_and_recv(
            &msg, offsetof(struct ac_io_message, cmd.raw) + msg.cmd.nbytes)) {
        #ifdef ICCX_DEBUG
        Serial.println("Starting queue loop failed");
            #endif
        return false;
    }    
    delay(200);
    return encrypted?iccx_key_exchange(node_id):true;
}

bool iccx_init(uint8_t node_id, bool encrypted)
{
    if (!iccx_queue_loop_start(node_id + 1, encrypted)) {
        return false;
    }

    return true;
}

static bool iccx_get_state(uint8_t node_id, iccx_state_t *state, bool encrypted)
{
    struct ac_io_message msg;

    msg.addr = node_id + 1;
    msg.cmd.code = ac_io_u16(encrypted? AC_IO_CMD_ICCx_FEL_POLL : AC_IO_CMD_ICCx_POLL);
    msg.cmd.nbytes = 1;
    /* buffer size of data we expect */
    msg.cmd.count = sizeof(iccx_state_t);

    if (!acio_send_and_recv(
            &msg, offsetof(struct ac_io_message, cmd.raw) + msg.cmd.count)) {
                      #ifdef ICCX_DEBUG
        Serial.print("Getting state of node ");
        Serial.print(node_id + 1);
        Serial.println(" failed");
            #endif
        return false;
    }
    
    /* got the data, decrypt it and check crc */
    if (encrypted)
    {
      crypto.crypt(msg.cmd.raw,18);
      #ifdef ICCX_DEBUG
      Serial.print("DECRYPTED : ");
      for (int i=0; i<18; i++)
      {
        if(msg.cmd.raw[i] < 0x10) Serial.print("0");
        Serial.print(msg.cmd.raw[i], HEX);
        Serial.print(" ");
      }
      Serial.println();
      #endif

      /* last two bytes are the CRC */
      uint16_t crc = msg.cmd.raw[16] << 8 | msg.cmd.raw[17];
      uint16_t crc_calc = crypto.CRCCCITT(msg.cmd.raw, 16);
      if (crc != crc_calc) {
        #ifdef ICCX_DEBUG
        Serial.print("INVALID CRC, received ");
        Serial.print(crc, HEX);
        Serial.print(" but calculated ");
        Serial.println(crc_calc, HEX);
        #endif        
        return false;
      }    
    }

    if (state != NULL) {
        memcpy(state, msg.cmd.raw, sizeof(iccx_state_t));
    }

    return true;
}

static bool iccx_read_card(uint8_t node_id, iccx_state_t *state, bool encrypted)
{
    struct ac_io_message msg;

    msg.addr = node_id + 1;
    if (encrypted)
    {
      msg.cmd.code = ac_io_u16(AC_IO_CMD_ICCx_FEL_ENGAGE);
      msg.cmd.nbytes = 4;
      static uint8_t fel_poll[4] = {0x00,0x03,0xFF,0xFF};
      memcpy(&msg.cmd.raw, fel_poll, 4);      
    }
    else
    {
      msg.cmd.code = ac_io_u16(AC_IO_CMD_ICCx_ENGAGE);
      msg.cmd.nbytes = 1;
      msg.cmd.count = sizeof(iccx_state_t);
    }
    
    if (!acio_send_and_recv(
            &msg, offsetof(struct ac_io_message, cmd.raw) + msg.cmd.count)) {
              #ifdef ICCX_DEBUG
        Serial.print("Reading card of node ");
        Serial.print(node_id + 1);
        Serial.println(" failed");
            #endif
        return false;
    }

    if (state != NULL) {
        memcpy(state, msg.cmd.raw, sizeof(iccx_state_t));
    }

    return true;
}


bool iccx_scan_card(uint8_t *type, uint8_t *uid, uint16_t *key_state, bool encrypted)
{
//icca_read_card suivi de get_state
 iccx_state_t state;
  #ifdef ICCX_DEBUG
   Serial.println("STEP1. CARD READ");
  #endif
  if (!iccx_read_card(0, &state, encrypted)){
  #ifdef ICCX_DEBUG
   Serial.println("cmd read card failed");
  #endif
  return false;
 }
  delay(200);
  
  #ifdef ICCX_DEBUG
   Serial.println("STEP2. GET STATE");
  #endif
 if (!iccx_get_state(0, &state, encrypted)){
  #ifdef ICCX_DEBUG
   Serial.println("cmd get state failed");
  #endif
  return false;
 }

/* copy data into type and uid*/
 #ifdef ICCX_DEBUG
   Serial.println("scan card success.");
   if (state.sensor_state == 2){
    if ((state.card_type&0x0F) == AC_IO_ICCx_CARD_TYPE_FELICA) Serial.print("FeliCa ");
    else Serial.print("ISO15693 ");
    Serial.println("card found!");
    Serial.print("UID =");
    for (int i=0; i<8; i++)
    {
      Serial.print(" ");
      if (state.uid[i] < 0x10) Serial.print("0");
      Serial.print(state.uid[i], HEX);
    }
    Serial.println();
    Serial.print("card type = ");
    Serial.println(state.card_type&0x0F);
   }
   else {
    Serial.print("no card found (status = ");
    Serial.print(state.sensor_state);
    Serial.println(")");
   }
  #endif

  *key_state = state.key_state;
  if (state.sensor_state == AC_IO_ICCx_SENSOR_CARD) {
  memcpy(uid, state.uid, 8);
  *type = (state.card_type&0x0F)+1;
  }
  else *type = 0;
  
  return true;
  
}
