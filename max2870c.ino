#include "MAX2870.h"

#define PIN_LE 4   // D2
#define PIN_CE 0   // D3
#define PIN_LD 5   // D1

#define MAX2870_reference_frequency_mhz 100
#define MAX2870_R_divider 2

bool blnk = false;
unsigned long times = millis();

String inputString = "";      // a String to hold incoming data
bool stringComplete = false;  // whether the string is complete

MAX2870 MAX2870(PIN_LE, PIN_CE, PIN_LD);

void setup() {

  pinMode(LED_BUILTIN, OUTPUT);
  Serial.begin(115200);

  delay(100);

  Serial.println("\nSPI MAX2870");

  MAX2870.powerOn(true);
  MAX2870.setPFD(MAX2870_reference_frequency_mhz, MAX2870_R_divider);
  MAX2870.set_RF_OUT_A(1200); 

  delay(2000);

  MAX2870.aout(2);
}

void loop() {
  if(millis() - times  >= 500){
    if (blnk)
      digitalWrite(LED_BUILTIN, HIGH);
    else
      digitalWrite(LED_BUILTIN, LOW); 

    times = millis();
    blnk = !blnk;
  }
  if (stringComplete) {
    Serial.print("Set freq:");
    Serial.println(inputString);
    MAX2870.set_RF_OUT_A(inputString.toInt()); 
    delay(10);
    Serial.print("loaded:");
    Serial.println(MAX2870.get_ld());
    inputString = "";
    stringComplete = false;
  }   
}


void serialEvent() {
  while (Serial.available()) {

    char inChar = (char)Serial.read();
    
    int tmp = int(inChar);
    if ((tmp>=48)&&(tmp<=57)) 
      inputString += inChar;

    if (inChar == '\n') {
      stringComplete = true;
    }
  }
}
