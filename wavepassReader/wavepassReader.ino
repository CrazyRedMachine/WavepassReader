
#include "ACIO.h"
#include "ICCx.h"
#define DEBUG

//#define WITH_USBHID

#ifdef WITH_USBHID
#include "Cardio.h"
#include <Keyboard.h>
Cardio_ Cardio;
#endif
#define PIN_EJECT_BUTTON 7

bool g_passthrough = false; // native mode (use arduino as simple TTL to USB)
bool g_encrypted = false; // FeliCa support and new readers (set to false for ICCA support, set to true otherwise)

void setup() {
  // put your setup code here, to run once:
 Serial.begin(57600);
 Serial1.begin(57600);
 pinMode(PIN_EJECT_BUTTON, INPUT_PULLUP);
/* TODO: check if passthrough */

if (!g_passthrough)
{
  delay(5000);
#ifdef DEBUG
  Serial.println("INIT CARDIO MODE");
#endif

#ifdef WITH_USBHID
  Keyboard.begin();
  Cardio.begin(false);
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

  /* CARDIO MODE */
  static unsigned long lastResult = 0;
  static uint32_t cooldown = 0;

  uint8_t uid[8] = {0,0,0,0,0,0,0,0};
  uint8_t type = 0;
  uint16_t keystate = 0;

  /* card eject button */
  if (digitalRead(PIN_EJECT_BUTTON)==LOW)
  {
    if (!g_encrypted)
      iccx_eject_card();
  }
  
  /* use acio commands to retrieve all info */
  if (millis()-lastResult < cooldown) return;
  cooldown = 0;
  if (!iccx_scan_card(&type,uid,&keystate,g_encrypted))
  {
#ifdef DEBUG
Serial.println("Error communicating with wavepass reader.");
#endif
  }
    /* KEYPAD */
  Serial.print("(mainloop) keystate = ");
  Serial.println(keystate,HEX);
     //everything is now in the keystate variable, we can use the masks in ICCx.h to parse
 if (keystate&ICCx_KEYPAD_MASK_0) Serial.println("0");
 if (keystate&ICCx_KEYPAD_MASK_00) Serial.println("00");
 if (keystate&ICCx_KEYPAD_MASK_EMPTY) Serial.println("empty");
 if (keystate&ICCx_KEYPAD_MASK_1) Serial.println("1");
 if (keystate&ICCx_KEYPAD_MASK_2) Serial.println("2");
 if (keystate&ICCx_KEYPAD_MASK_3) Serial.println("3");
 if (keystate&ICCx_KEYPAD_MASK_4) Serial.println("4");
 if (keystate&ICCx_KEYPAD_MASK_5) Serial.println("5");
 if (keystate&ICCx_KEYPAD_MASK_6) Serial.println("6");
 if (keystate&ICCx_KEYPAD_MASK_7) Serial.println("7");
 if (keystate&ICCx_KEYPAD_MASK_8) Serial.println("8");
 if (keystate&ICCx_KEYPAD_MASK_9) Serial.println("9");

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
    Cardio.setUID(type, uid);
    Cardio.sendState();
#endif
    lastResult = millis();
    cooldown = 0; //no cooldown cause I still need to read the keypad
    return;
  }
  lastResult = millis();
  cooldown = 0; //no cooldown cause I still need to read the keypad
  
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
