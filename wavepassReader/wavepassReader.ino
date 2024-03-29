#include "ACIO.h"
#include "ICCx.h"
//#define DEBUG

#define WITH_USBHID

#ifdef WITH_USBHID
#define USB_HID_COOLDOWN 3000
#include "Cardio.h"
#include <Keyboard.h>
Cardio_ Cardio;
#endif

/* ICCA-only (slotted) options */
#define PIN_EJECT_BUTTON 7
#define KEYPAD_BLANK_EJECT 1 //make blank key from keypad eject currently inserted card (ICCA only)
//#define SEND_ONLY_ONCE //send uid only once per i
#define AUTO_EJECT_TIMER 0 //auto eject valid cards after a set delay (in ms), 0 to disable (note: must be smaller than USB_HID_COOLDOWN)

//#define PRESS_KEY_ON_BOOT //press a key on boot (useful for some motherboards)
#define PRESS_KEY_TIMER 5000
#define PRESS_KEY_DURATION 500
#define PRESS_KEY KEY_F1

bool g_passthrough = false; // native mode (use arduino as simple TTL to USB)
bool g_encrypted = true; // FeliCa support and new readers (set to false for ICCA support, set to true otherwise)

static char g_keypad_code[12] = 
{'0', ',', '\337',
 '1', '2', '3', 
 '4', '5', '6', 
 '7', '8', '9'};

static int g_keypad_mask[12] = 
{ICCx_KEYPAD_MASK_0, ICCx_KEYPAD_MASK_00, ICCx_KEYPAD_MASK_EMPTY, 
 ICCx_KEYPAD_MASK_1, ICCx_KEYPAD_MASK_2, ICCx_KEYPAD_MASK_3, 
 ICCx_KEYPAD_MASK_4, ICCx_KEYPAD_MASK_5, ICCx_KEYPAD_MASK_6, 
 ICCx_KEYPAD_MASK_7, ICCx_KEYPAD_MASK_8, ICCx_KEYPAD_MASK_9};

#ifdef DEBUG
static const char* g_keypad_printable[12] = 
{"0", "00", "empty",
 "1", "2", "3", 
 "4", "5", "6", 
 "7", "8", "9"};
#endif

void setup() {
  // put your setup code here, to run once:
 Serial.begin(57600);
 Serial1.begin(57600);
 pinMode(PIN_EJECT_BUTTON, INPUT_PULLUP);
/* TODO: check if passthrough */

if (!g_passthrough)
{
  delay(2000);
#ifdef DEBUG
  Serial.println("INIT CARDIO MODE");
#endif

#ifdef WITH_USBHID
  Keyboard.begin();
  Cardio.begin(false);
  #ifdef PRESS_KEY_ON_BOOT
  delay(PRESS_KEY_TIMER);
  Keyboard.press(PRESS_KEY);
  delay(PRESS_KEY_DURATION);
  Keyboard.releaseAll();
  #endif
#endif

  delay(1000);
#ifdef DEBUG
Serial.println("acio open");
#endif
acio_open();
  delay(1000);
#ifdef DEBUG
Serial.println("iccx init");
#endif
  iccx_init(0, g_encrypted);
#ifdef DEBUG
Serial.println("init all done, entering main loop.");
#endif
}

}

void loop() {

  /* NATIVE MODE */
  if (g_passthrough)
  {
    passthrough_loop();
    return;
  }

#ifdef WITH_USBHID
  static unsigned long lastReport = 0;
#endif
  /* CARDIO MODE */
  uint8_t uid[8] = {0,0,0,0,0,0,0,0};
  uint8_t type = 0;
  uint16_t keystate = 0;
  static uint16_t prev_keystate = 0;
#ifdef SEND_ONLY_ONCE
  static bool can_send = true;
#endif

  /* card eject button */
  if (!g_encrypted && (digitalRead(PIN_EJECT_BUTTON)==LOW))
  {
      iccx_eject_card(AC_IO_ICCA_SLOT_STATE_OPEN);
#ifdef SEND_ONLY_ONCE
      can_send = true;
#endif
  }

#if AUTO_EJECT_TIMER > 0
  static bool already_eject = false;
  if (!g_encrypted && !already_eject && ((millis()-lastReport) >= AUTO_EJECT_TIMER))
  {
      iccx_eject_card(AC_IO_ICCA_SLOT_STATE_OPEN);
      already_eject = true;
#ifdef SEND_ONLY_ONCE
      can_send = true;
#endif
  }

#endif
  
  /* use acio commands to retrieve all info */
  if (!iccx_scan_card(&type,uid,&keystate,g_encrypted))
  {
#ifdef DEBUG
Serial.println("Error communicating with wavepass reader.");
#endif
  }
    /* KEYPAD */
     //everything is now in the keystate variable, we can use the masks in ICCx.h to parse

 #ifdef KEYPAD_BLANK_EJECT
  if (!g_encrypted && (keystate&ICCx_KEYPAD_MASK_EMPTY))
  {
    iccx_eject_card(AC_IO_ICCA_SLOT_STATE_OPEN);
    #ifdef SEND_ONLY_ONCE
    can_send = true;
    #endif
  }
 #endif
 
 for (int i=0; i<12; i++)
 {
  check_key(i, keystate, prev_keystate);
 }
 prev_keystate = keystate;
 
    /* CARD */  
 
  if (type)
  {
    
#ifdef DEBUG
Serial.print("Found a card of type ");
if (type == 1) Serial.print("ISO15693");
else Serial.print("FeliCa");
Serial.print(" with uid =");
for (int i=0; i<8; i++)
    {
      Serial.print(" ");
      if (uid[i] < 0x10) Serial.print("0");
      Serial.print(uid[i], HEX);
    }
    Serial.println();
#endif

#ifdef WITH_USBHID 
  if (millis()-lastReport < USB_HID_COOLDOWN) return;
#ifdef SEND_ONLY_ONCE
  if (!g_encrypted && !can_send) return;
#endif
    Cardio.setUID(type, uid);
    Cardio.sendState();
    lastReport = millis();
#if AUTO_EJECT_TIMER > 0    
    already_eject = false;
#endif
#ifdef SEND_ONLY_ONCE
    can_send = false;
#endif
#endif
  }
  
}

/* Simple serial passthrough for native mode */
void passthrough_loop() {
  while (Serial.available()) {      // If anything comes in Serial (USB),
    Serial1.write(Serial.read());   // read it and send it out Serial1 (pins 0 and 1)
  }
  delay(5);
  while (Serial1.available()) {     // If anything comes in Serial1 (USB)
    Serial.write(Serial1.read());   // read it and send it out Serial (pins 0 and 1)
  }
}


#define IS_PRESSED(x) ((keystate&x)&&(!(prev_keystate&x)))
#define IS_RELEASED(x) ((!(keystate&x))&&(prev_keystate&x))
static void check_key(uint8_t i, int keystate, int prev_keystate)
{

 if (IS_PRESSED(g_keypad_mask[i]))
 {
  #ifdef DEBUG
    Serial.println(g_keypad_printable[i]);
  #endif
#ifdef WITH_USBHID
    Keyboard.press(g_keypad_code[i]);
 }
 else if (IS_RELEASED(g_keypad_mask[i]))
 {
    Keyboard.release(g_keypad_code[i]);
#endif  
 }

}
