#include <Wire.h>
#include <SPI.h>
#include <Adafruit_PN532.h>
#define LED1 7
#define LED2 8
#define SENSOR_A A0
#define SENSOR_B A1
#define VIBRATE A3
#define POINT 6
#define THRESH_A 500
#define THRESH_B 500
#define DELAY_TIME 250
#define COUNT_THRESH 40
#define RFID 9
#define PN532_SCK  (2)
#define PN532_MOSI (3)
#define PN532_SS   (4)
#define PN532_MISO (5)
#define PN532_IRQ   (2)
#define PN532_RESET (3)  // Not connected by default on the NFC Shield

Adafruit_PN532 nfc(PN532_SCK, PN532_MISO, PN532_MOSI, PN532_SS);

#if defined(ARDUINO_ARCH_SAMD)
// for Zero, output on USB Serial console, remove line below if using programming port to program the Zero!
// also change #define in Adafruit_PN532.cpp library file
   #define Serial SerialUSB
#endif

enum STATE_enum {WAIT, AT_A, BETWEEN_AB, AT_B, PASSED_B, AT_BOTH, TRASH_IN, FULL};
STATE_enum state;
bool a_blocked;
bool b_blocked;
int counter;
uint8_t success;
uint8_t uid[] = { 0, 0, 0, 0, 0, 0, 0 };  // Buffer to store the returned UID
uint8_t uidLength;                        // Length of the UID (4 or 7 bytes depending on ISO14443A card type)

void setup(void) {
  // put your setup code here, to run once:
  pinMode(LED1, OUTPUT);
  pinMode(LED2, OUTPUT);
  pinMode(POINT, OUTPUT);
  pinMode(VIBRATE, INPUT);
  pinMode(SENSOR_A, INPUT);
  pinMode(SENSOR_B, INPUT);
  pinMode(RFID, INPUT);
  pinMode(PN532_SCK, OUTPUT);
  pinMode(PN532_MOSI, OUTPUT);
  pinMode(PN532_SS, OUTPUT);
  pinMode(PN532_MISO, INPUT);
  digitalWrite(LED1, HIGH);
  digitalWrite(LED2, HIGH);
  state = WAIT;
  counter = 0;

  
  #ifndef ESP8266
    while (!Serial); // for Leonardo/Micro/Zero
  #endif
  Serial.begin(115200);
  Serial.println("Hello!");

  nfc.begin();

  uint32_t versiondata = nfc.getFirmwareVersion();
  if (! versiondata) {
    Serial.print("Didn't find PN53x board");
    while (1); // halt
  }
  // Got ok data, print it out!
  Serial.print("Found chip PN5"); Serial.println((versiondata>>24) & 0xFF, HEX); 
  Serial.print("Firmware ver. "); Serial.print((versiondata>>16) & 0xFF, DEC); 
  Serial.print('.'); Serial.println((versiondata>>8) & 0xFF, DEC);
  // configure board to read RFID tags
  nfc.SAMConfig();
  Serial.println("Waiting for an ISO14443A Card ...");
}

void loop(void) {
  success = nfc.readPassiveTargetID(PN532_MIFARE_ISO14443A, uid, &uidLength);
  if (success) {
    // put your main code here, to run repeatedly:
    a_blocked = analogRead(SENSOR_A) < THRESH_A;
    b_blocked = analogRead(SENSOR_B) < THRESH_B;
    
    Serial.print("LED A: ");
    Serial.print(analogRead(SENSOR_A));
    Serial.print("\tLED B: ");
    Serial.print(analogRead(SENSOR_B));
    Serial.print("\tState: ");
    print_state();
    run_fsm();
  
  }
  delay(DELAY_TIME);
}

void run_fsm(){
  switch(state){
    case WAIT:
      digitalWrite(POINT, 0);
      if(a_blocked){
        state = AT_A;
      }
      break;
      
    case AT_A:
      if(a_blocked){
        if(b_blocked){
          state = AT_BOTH;
        }
      }
      else if(b_blocked){
        state = AT_B;
      }
      else{
        state = BETWEEN_AB;
      }
      break;
      
    case BETWEEN_AB:
      if(b_blocked){
        state = AT_B;
      }
      break;
    
    case AT_B:
      if(a_blocked){
        state = AT_BOTH;
      }
      else if(!b_blocked){
        state = PASSED_B;
      }
      break;
    
    case PASSED_B:
      state = TRASH_IN;
      break;
    
    case AT_BOTH:
      if(a_blocked && !b_blocked){
        state = AT_A;
      }
      else if(!a_blocked && b_blocked){
        state = AT_B;
      }
      if(counter > COUNT_THRESH){
        counter = 0;
        digitalWrite(POINT, 1);
        state = FULL;
      }
      counter += 1;
      break;
    
    case TRASH_IN:
      
      if(counter > COUNT_THRESH && !a_blocked && !b_blocked){
        counter = 0;
        digitalWrite(POINT, 1);
        state = WAIT;
      }
      counter += 1;
      break;
    
    case FULL:
      digitalWrite(POINT, 0);

      break;
    
  }
  
}

void print_state(){
  switch(state){
    case WAIT: 
      Serial.println("WAIT");
      break;
    case AT_A:  
      Serial.println("AT_A");
      break;
    case BETWEEN_AB: 
      Serial.println("BETWEEN_AB");
      break;
    case AT_B: 
      Serial.println("AT_B");
      break;
    case PASSED_B: 
      Serial.println("PASSED_B");
      break;
    case AT_BOTH: 
      Serial.println("AT_BOTH");
      break;
    case TRASH_IN: 
      Serial.println("TRASH_IN");
      break;
    case FULL: 
      Serial.println("FULL");
      break;
  }
}
