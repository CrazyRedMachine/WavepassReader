#include "ACIO.h"
#include "ICCx.h"

//#define WITH_USBHID

#ifdef WITH_USBHID
#include "Cardio.h"
#include <Keyboard.h>
Cardio_ Cardio;
#endif

bool g_passthrough = false;

void setup() {
  // put your setup code here, to run once:
 Serial.begin(57600);
 Serial1.begin(57600);

/* TODO: check if passthrough */

if (!g_passthrough)
{
  delay(5000);
  Serial.println("INIT CARDIO MODE");

#ifdef WITH_USBHID
  Keyboard.begin();
  Cardio.begin(false);
#endif

  delay(1000);
  Serial.println("acio open");
  acio_open();
  delay(1000);
  Serial.println("iccx init");
  iccx_init(0);
  Serial.println("init all done, entering main loop.");
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
  static uint32_t cardBusy = 0;
  
    /* KEYPAD */
      /* TODO: use acio commands to retrieve keypad state */


    /* CARD SCAN */
  if (millis()-lastResult < cardBusy) return;
  
  cardBusy = 0;
  uint8_t uid[8] = {0,0,0,0,0,0,0,0};
  uint8_t type = 0;

  /* use acio commands to fill type and uid */
  /* iccx_scan_card(&type,uid) */
  iccx_scan_card(&type,uid);
 
  if (type)
  {
#ifdef WITH_USBHID 
    Cardio.setUID(type, uid);
    Cardio.sendState();
#endif
    lastResult = millis();
    cardBusy = 3000;
    return;
  }
  lastResult = millis();
  cardBusy = 200;
  
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
