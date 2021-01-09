#include "ICCx.h"
#define ICCX_DEBUG

static bool iccx_queue_loop_start(uint8_t node_id)
{
    struct ac_io_message msg;

    msg.addr = node_id;
    msg.cmd.code = ac_io_u16(AC_IO_CMD_ICCx_QUEUE_LOOP_START);
    msg.cmd.nbytes = 1;
    msg.cmd.status = 0;

    if (!acio_send_and_recv(
            &msg, offsetof(struct ac_io_message, cmd.raw) + 1)) {
        #ifdef ICCX_DEBUG
        Serial.println("Starting queue loop failed");
            #endif
        return false;
    }
#ifdef ICCX_DEBUG
Serial.print("Started queue loop of node ");
Serial.print(node_id);
Serial.print(", status: ");
Serial.println(msg.cmd.status);
            #endif

    return true;
}

bool iccx_init(uint8_t node_id)
{
    if (!iccx_queue_loop_start(node_id + 1)) {
        return false;
    }

    return true;
}

static bool iccx_get_state(uint8_t node_id, iccx_state_t *state)
{
    struct ac_io_message msg;

    msg.addr = node_id + 1;
    msg.cmd.code = ac_io_u16(AC_IO_CMD_ICCx_POLL);
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

    if (state != NULL) {
        memcpy(state, msg.cmd.raw, sizeof(iccx_state_t));
    }

    return true;
}

static bool iccx_read_card(uint8_t node_id, iccx_state_t *state)
{
    struct ac_io_message msg;

    msg.addr = node_id + 1;
    msg.cmd.code = ac_io_u16(AC_IO_CMD_ICCx_ENGAGE);
    msg.cmd.nbytes = 1;
    /* buffer size of data we expect */
    msg.cmd.count = sizeof(iccx_state_t);

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


bool iccx_scan_card(uint8_t *type, uint8_t *uid)
{
//icca_read_card suivi de get_state
 iccx_state_t state;
  #ifdef ICCX_DEBUG
   Serial.println("STEP1. CARD READ");
  #endif
  if (!iccx_read_card(0, &state)){
  #ifdef ICCX_DEBUG
   Serial.println("cmd read card failed");
  #endif
  return false;
 }

  #ifdef ICCX_DEBUG
   Serial.println("STEP2. GET STATE");
  #endif
 if (!iccx_get_state(0, &state)){
  #ifdef ICCX_DEBUG
   Serial.println("cmd get state failed");
  #endif
  return false;
 }

/* copy data into type and uid*/
 #ifdef ICCX_DEBUG
   Serial.println("scan card success.");
   if (state.status_code == 2){
    if (state.card_type) Serial.print("FeliCa ");
    else Serial.print("ISO15693 ");
    Serial.println("card found!");
    Serial.print("UID = ");
    for (int i=0; i<8; i++)
    {
      Serial.print(" ");
      if (state.uid[i] < 0x10) Serial.print("0");
      Serial.print(state.uid[i], HEX);
    }
    Serial.println();
    Serial.print("card type = ");
    Serial.println(state.card_type);
   }
   else {
    Serial.print("no card found (status = ");
    Serial.print(state.status_code);
    Serial.println(")");
   }
  #endif

  if (state.status_code != 2) 
    return false;

  memcpy(uid, state.uid, 8);
  *type = state.card_type+1;
  return true;
  
}
