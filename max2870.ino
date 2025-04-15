#include <SPI.h>

#define PIN_LE 4   // D2
#define PIN_SS 0   // D3

bool blnk = false;
unsigned long times = millis();

const uint32_t MOD = 4000;            // MOD — максимальне значення
const uint64_t F_REF = 100000000;     // 100 MHz референс
const uint8_t R_DIV = 2;              // дільник референсу

String inputString = "";      // a String to hold incoming data
bool stringComplete = false;  // whether the string is complete

void pulseLE() {
  digitalWrite(PIN_LE, HIGH);
  delayMicroseconds(1);
  digitalWrite(PIN_LE, LOW);
  delayMicroseconds(1);
}

void writeRegister(uint32_t data) {
  for (int i = 3; i >= 0; i--) {
    SPI.transfer((data >> (i * 8)) & 0xFF);
  }
  pulseLE();
  //delayMicroseconds(1);
}

uint8_t get_diva(uint64_t freq_kHz) 
{
  if (freq_kHz >= 3000000) return 0;
  if (freq_kHz >= 1500000) return 1;
  if (freq_kHz >=  750000) return 2;
  if (freq_kHz >=  375000) return 3;
  if (freq_kHz >=  187500) return 4;
  if (freq_kHz >=   93750) return 5;
  if (freq_kHz >=   46875) return 6;
  return 7;
}

void setFrequency(uint64_t freq_kHz) {

  if (freq_kHz < 23500) return;
  if (freq_kHz > 6000000) return;

  uint64_t f_pfd = F_REF / R_DIV;

  uint8_t diva = get_diva(freq_kHz); 
  double ratio = (double)freq_kHz * 1000 * powf(2, diva) / f_pfd;
  uint32_t INT = (uint32_t)ratio;
  uint32_t FRAC = (uint32_t)((ratio - INT) * MOD);
  uint32_t bs = f_pfd / 50000;
  if (bs > 1023) {
    bs = 1023;
  }
  uint32_t cdiv = f_pfd / 100000;

  uint8_t lds = 0;
  if (f_pfd > 32000000) {
    lds = 1;
  }

  // --- Формуємо регістри ---
  uint32_t R0 = (0 << 31) |(INT << 15) | (FRAC << 3) | 0;
  uint32_t R1 = (1 << 29) | // CPL
                (1 << 15) | //phase
                (MOD << 3) | 1;

  // R2 – включає PFD, R divider
  uint32_t R2 = (lds << 31) | 
                (0 << 26) |  // MUXOUT = GVdd
                (0 << 25) |      // DBR = 0
                (0 << 24) |      // RDIV2 = 0
                (R_DIV << 14) |  // R counter
                (0 << 13) |      // REG4DB = 0
                (0 << 9) |      // CP = 0.96
                (0 << 8) |      // LDF = 1
                (0 << 7) |      // LDP = 0
                (1 << 6) |      // PDP = 1
                (0 << 5) |      // SHDN = 0
                (0 << 4) |      // TRI = 0
                (0 << 3) |      // RST = 0
                2;

  uint32_t R3 = (0 << 26) |  //VCO
                (0 << 25) |  //VAS SHDN = 0;  ensbled
                (0 << 24) |  //VAS RETUNE = 0 
                //(1 << 17) |  //CDM
                (0 << 15) |  //CDM
                (cdiv << 3) |
                3;  

  uint32_t R4 = (0x18 << 26) |
                (((bs >> 8) & 0x03) << 24) |  //BS
                (1 << 23) |  //FB = fundamental
                (diva << 20) |  //DIVA
                ((bs & 0xff) << 12) |  //BS
                (0 << 9) |  //BDIV
                (0 << 8) |  //RFOUTB disable
                (0 << 6) |  //RFOUTB = +5dBm
                (1 << 5) |  //RFOUTA enable
                (3 << 3) |  //RFOUTA = +5dBm
                4 ;   // включає RF вихід, 50 Ω
  uint32_t R5 = 0x400005;  // стандартний

  // --- Відправляємо регістри ---
  writeRegister(R5);
  writeRegister(R4);
  writeRegister(R3);
  writeRegister(R2);
  writeRegister(R1);
  writeRegister(R0);

  Serial.print("Частота встановлена: ");
  Serial.print(freq_kHz/1000.0,3);
  Serial.println(" MHz");
/*
  Serial.print("f_pfd:");
  Serial.print(f_pfd);
  Serial.print(" ratio:");
  Serial.print(ratio, 3);
  Serial.print(" INT:");
  Serial.print(INT);
  Serial.print(" bs:");
  Serial.print(bs);
  Serial.print(" cdiv:");
  Serial.print(cdiv);
  Serial.print(" diva:");
  Serial.print(diva);
  Serial.print(" FRAC:");
  Serial.println(FRAC);

  Serial.print("REG0: "); Serial.println(R0, HEX);
  Serial.print("REG1: "); Serial.println(R1, HEX);
  Serial.print("REG2: "); Serial.println(R2, HEX);
  Serial.print("REG3: "); Serial.println(R3, HEX);
  Serial.print("REG4: "); Serial.println(R4, HEX);
  Serial.print("REG5: "); Serial.println(R5, HEX);
*/
}

void init_0()
{
  writeRegister(5);
  delayMicroseconds(20);
  writeRegister(4);
  writeRegister(3);
  writeRegister(2);
  writeRegister(1);
  writeRegister(0);

  writeRegister(5);
  writeRegister(4);
  writeRegister(3);
  writeRegister(2);
  writeRegister(1);
  writeRegister(0);

  delay(100);
}

void setup() {
  pinMode(LED_BUILTIN, OUTPUT);
  Serial.begin(115200);
  delay(100);
  //while (!Serial);             // Leonardo: wait for serial monitor
  Serial.println("\nSPI MAX2870");

  pinMode(PIN_LE, OUTPUT);
  pinMode(PIN_SS, OUTPUT);
  digitalWrite(PIN_SS, HIGH);
  digitalWrite(PIN_LE, LOW);


  SPI.begin();
  SPI.setClockDivider(SPI_CLOCK_DIV8);  // 10 MHz
  SPI.setDataMode(SPI_MODE0);
  SPI.setBitOrder(MSBFIRST);

  delay(100);

  init_0();

  // Приклад: встановити 433.92 МГц
  //setFrequency(433920);
  //setFrequency(1500000);
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
    Serial.println("Set freq:");
    Serial.println(inputString);
    setFrequency(inputString.toInt());
    delay(10);
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