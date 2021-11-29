#include <EEPROM.h>     
#include <SPI.h>
#include <MFRC522.h>
#include <LiquidCrystal.h>
#define redLed A4
#define greenLed A5
#define buzzer A3
bool programMode = false;
String uid;
int lastPos;
String cards[10] = {}; // array that stores uid cards
String status[10] = {}; // array that stores their status
String users[10] = {"Mr. Jozzy", "Mr. GDee", "Mr. Oselu"}; // array that stores their names
uint8_t successRead;    // Variable integer to keep if we have Successful Read from Reader
byte storedCard[4];   // Stores an ID read from EEPROM
byte readCard[4];   // Stores scanned ID read from RFID Module
byte masterCard[4];   // Stores master card's ID read from EEPROM
#define SS_PIN 10
#define RST_PIN 9
MFRC522 mfrc522(SS_PIN, RST_PIN);

const int rs = 2, en = 3, d4 = 4, d5 = 5, d6 = 6, d7 = 7;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);
void setup() {
  pinMode(redLed, OUTPUT);
  pinMode(greenLed, OUTPUT);
  pinMode(buzzer, OUTPUT);
  digitalWrite(buzzer, 1);
  lcd.begin(16, 2);
  for(int y = 0; y < 3; ++y)
  {
    digitalWrite(greenLed, 1);
    delay(300);
    digitalWrite(greenLed, 0);
    delay(300);
  }
  digitalWrite(buzzer, 0);
  lcd.setCursor(0, 0);
  lcd.print("RFID Attendance");
  delay(200);
  lcd.setCursor(0, 1);
  lcd.print("Initializing");
  for(int y = 0; y < 4; y++)
  {
    lcd.setCursor(12 + y, 1);
    lcd.print('.');
    delay(1000);
  }
  lcd.clear();
  Serial.begin(9600);
  SPI.begin();
  mfrc522.PCD_Init();
  //mfrc522.PCD_SetAntennaGain(mfrc522.RxGain_max);
  ShowReaderDetails();
  if (EEPROM.read(1) != 143) {
    Serial.println(F("No Master Card Defined"));
    Serial.println(F("Scan A PICC to Define as Master Card"));
    lcd.setCursor(0, 0);
    lcd.print("Place a Card to");
    lcd.setCursor(0, 1);
    lcd.print("make it master");
    do {
      successRead = getID();  
      for(int y = 0; y < 2; ++y)
      {
        lcd.setCursor(14 + y, 1);
        lcd.print('.');
        delay(250);      
      }
      for(int y = 0; y < 2; ++y)
      {
        lcd.setCursor(14 + y, 1);
        lcd.print(' ');    
      }
    }while (!successRead);
    lcd.clear();
    for ( uint8_t j = 0; j < 4; j++ ) {       
      EEPROM.write( 2 + j, readCard[j] );
    }
    EEPROM.write(1, 143);
    Serial.println(F("Master Card Set"));
    lcd.setCursor(0, 0);
    lcd.print("MasterCard Added");
    lcd.setCursor(0, 1);
    lcd.print("Succesfully!");
    delay(800);
  }
  String id;
  Serial.println(F("-------------------"));
  Serial.println(F("Master Card's UID"));
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("MasterCard Id: ");
  lcd.setCursor(0, 1);
  for ( uint8_t i = 0; i < 4; i++ ) {
    masterCard[i] = EEPROM.read(2 + i);
    Serial.print(masterCard[i], HEX);
    id += String(masterCard[i], HEX);
  }
  lcd.print(id);
  delay(1000);
  Serial.println("");
  Serial.println(F("-------------------"));
  Serial.println(F("Everything is ready"));
  Serial.println(F("Waiting PICCs to be scanned"));
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Device Ready");
  delay(500);
  lcd.clear();
}
void loop () {
  do {
    successRead = getID();
    if (programMode) {
      lcd.setCursor(0, 0);
      lcd.print(">>Program Mode<<");
      lcd.setCursor(0, 1);
      lcd.print("Place a Card:");
      cycleLeds();
    }
    else {
      normalModeOn();
      lcd.setCursor(0, 0);
      lcd.print("Place a Card: ");
      lcd.setCursor(0, 1);
      lcd.print("                ");
    }
  }while (!successRead);
  if (programMode) {
    if (isMaster(readCard)) {
      Serial.println(F("Master Card Scanned"));
      Serial.println(F("Exiting Program Mode"));
      Serial.println(F("-----------------------------"));
      programMode = false;
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Exiting Program");
      lcd.setCursor(0, 1);
      lcd.print("Mode");
      for(int y = 0; y < 3; ++y)
      {
        lcd.setCursor(4 + y, 1);
        lcd.print('.');
        delay(250);      
      }
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Place a Card: ");
      lcd.setCursor(0, 1);
      lcd.print("                ");
      return;
    }
    else {
      if (findID(readCard)) {
        Serial.println(F("PICC recognised, removing..."));
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Card recognised");
        lcd.setCursor(0, 1);
        lcd.print("Removing");
        for(int y = 0; y < 8; ++y)
        {
          lcd.setCursor(8 + y, 1);
          lcd.print('.');
          delay(250);      
        }
        deleteID(readCard);
        Serial.println("-----------------------------");
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Card Removed");
        lcd.setCursor(0, 1);
        lcd.print("Succesfully!");
        delay(800);
        Serial.println(F("Scan a PICC to ADD or REMOVE from memory"));
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Place a Card: ");
        lcd.setCursor(0, 1);
        lcd.print("                ");
      }
      else {                   
        Serial.println(F("New PICC, adding..."));
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("New Card Scanned");
        lcd.setCursor(0, 1);
        lcd.print("Adding");
        for(int y = 0; y < 10; ++y)
        {
          lcd.setCursor(6 + y, 1);
          lcd.print('.');
          delay(250);      
        }
        writeID(readCard);
        Serial.println(F("-----------------------------"));
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Card Added");
        lcd.setCursor(0, 1);
        lcd.print("Succesfully!");
        delay(800);
        Serial.println(F("Scan a PICC to ADD or REMOVE from memory"));
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Place a Card: ");
        lcd.setCursor(0, 1);
        lcd.print("                ");
      }
    }
  }
  else {
    if (isMaster(readCard)) {
      programMode = true;
      Serial.println(F("Master Card Detected - Entered Program Mode"));
      uint8_t count = EEPROM.read(0);
      Serial.print(F("Found "));
      Serial.print(count);
      Serial.print(F(" record(s) on EEPROM"));
      Serial.println("");
      Serial.println(F("Scan a PICC to ADD or REMOVE to EEPROM"));
      Serial.println(F("Scan Master Card again to Exit Program Mode"));
      Serial.println(F("-----------------------------"));
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Place a Card: ");
      lcd.setCursor(0, 1);
      lcd.print("                ");
    }
    else {
      if (findID(readCard)) {
        int index = lookUp(cards, sizeof(cards), compileId(readCard, sizeof(readCard)));
        Serial.println("Index is " + String(index));
        if(!index)
        {
          int pos = lastPos;
          cards[pos] = compileId(readCard, sizeof(readCard));
          status[pos] = "signedIn";
          Serial.print(F("Welcome! "));
          Serial.println(compileId(readCard, sizeof(readCard)));
          lcd.clear();
          lcd.setCursor(0, 0);
          lcd.print("#ACCESS GRANTED#");
          lcd.setCursor(0, 1);
          delay(500);
          lcd.print("ID: " + compileId(readCard, sizeof(readCard)));
          delay(500);
          lcd.setCursor(0, 1);
          lcd.print("Welcome" + users[pos]);
          granted(300);
          lastPos++;
          delay(2000);
          lcd.setCursor(0, 0);
          lcd.print("Place a Card:   ");
          lcd.setCursor(0, 1);
          lcd.print("                ");
        }
        else if(index > 0 && status[index - 1] == "signedIn") {
          lcd.clear();
          lcd.setCursor(0, 0);
          lcd.print("#ACCESS GRANTED#");
          lcd.setCursor(0, 1);
          delay(500);
          lcd.print("ID: " + compileId(readCard, sizeof(readCard)));
          delay(500);
          lcd.setCursor(0, 1);
          lcd.print("Bye " + users[index-1]);
          granted(300);
          status[index - 1] = "signedOut";
          delay(2000);
          lcd.setCursor(0, 0);
          lcd.print("Place a Card:   ");
          lcd.setCursor(0, 1);
          lcd.print("                ");
        }
        else if(index > 0 && status[index - 1] == "signedOut")
        {
          lcd.clear();
          lcd.setCursor(0, 0);
          lcd.print("#ACCESS GRANTED#");
          lcd.setCursor(0, 1);
          delay(500);
          lcd.print("ID: " + compileId(readCard, sizeof(readCard)));
          delay(500);
          lcd.setCursor(0, 1);
          lcd.print("Welcome" + users[index-1]);
          granted(300);
          status[index - 1] = "signedIn";
          delay(2000);
          lcd.setCursor(0, 0);
          lcd.print("Place a Card:   ");
          lcd.setCursor(0, 1);
          lcd.print("                ");
        }
      }
      else {
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("#ACCESS DENIED#");
        lcd.setCursor(0, 1);
        delay(500);
        lcd.print("I don't know you");
        Serial.println(F("I don't know you"));
        denied();
        delay(2000);
        lcd.setCursor(0, 0);
        lcd.print("Place a Card:   ");
        lcd.setCursor(0, 1);
        lcd.print("                ");
      }
    }
  }
}
void granted ( uint16_t setDelay) {
  for(int x = 0; x < 3; ++x)
  {
    digitalWrite(buzzer, 1);
    delay(50);
    digitalWrite(buzzer, 0);
    delay(50);
  }
  digitalWrite(redLed, 0);
  digitalWrite(greenLed, 1);
}
void denied() {
  for(int x = 0; x < 2; ++x)
  {
    digitalWrite(buzzer, 1);
    delay(200);
    digitalWrite(buzzer, 0);
    delay(100);
  }
  digitalWrite(greenLed, 0);  // Make sure green LED is off
  digitalWrite(redLed, 1);   // Turn on red LED
}
uint8_t getID() {
  if (!mfrc522.PICC_IsNewCardPresent()) {
    return 0;
  }
  if (!mfrc522.PICC_ReadCardSerial()) {
    return 0;
  }
  Serial.println(F("Scanned PICC's UID:"));
  for ( uint8_t i = 0; i < 4; i++) {  
    readCard[i] = mfrc522.uid.uidByte[i];
    Serial.print(readCard[i], HEX);
  }
  Serial.println("");
  mfrc522.PICC_HaltA();
  return 1;
}

void ShowReaderDetails() {
  byte v = mfrc522.PCD_ReadRegister(mfrc522.VersionReg);
  Serial.print(F("MFRC522 Software Version: 0x"));
  Serial.print(v, HEX);
  if (v == 0x91)
    Serial.print(F(" = v1.0"));
  else if (v == 0x92)
    Serial.print(F(" = v2.0"));
  else
    Serial.print(F(" (unknown),probably a chinese clone?"));
  Serial.println("");
  if ((v == 0x00) || (v == 0xFF)) {
    Serial.println(F("WARNING: Communication failure, is the MFRC522 properly connected?"));
    Serial.println(F("SYSTEM HALTED: Check connections."));
    // Visualize system is halted
    digitalWrite(greenLed, 0);  // Make sure green LED is off
    digitalWrite(redLed, 1);   // Turn on red LED
    while (true); // halt
  }
}
void cycleLeds() {
  digitalWrite(redLed, 0);  
  digitalWrite(greenLed, 1);  
  delay(200);
  digitalWrite(redLed, 1);  
  digitalWrite(greenLed, 0);    
}
void normalModeOn () {
  digitalWrite(redLed, 1);
  digitalWrite(greenLed, 0);
}

void readID( uint8_t number ) {
  uint8_t start = (number * 4 ) + 2;    // Figure out starting position
  for ( uint8_t i = 0; i < 4; i++ ) {     // Loop 4 times to get the 4 Bytes
    storedCard[i] = EEPROM.read(start + i);   // Assign values read from EEPROM to array
  }
}
void writeID( byte a[] ) {
  if ( !findID( a ) ) {
    uint8_t num = EEPROM.read(0);    
    uint8_t start = ( num * 4 ) + 6;
    num++;               
    EEPROM.write( 0, num );
    for ( uint8_t j = 0; j < 4; j++ ) {
      EEPROM.write( start + j, a[j] );
    }
    successWrite();
    Serial.println(F("Succesfully added ID record to memory"));
  }
  else {
    failedWrite();
    Serial.println(F("Failed! There is something wrong with ID or memory"));
  }
}
void deleteID( byte a[] ) {
  if ( !findID( a ) ) {
    failedWrite();     
    Serial.println(F("Failed! There is something wrong with ID or bad EEPROM"));
  }
  else {
    uint8_t num = EEPROM.read(0);  
    uint8_t slot;      
    uint8_t start;     
    uint8_t looping;    
    uint8_t j;
    uint8_t count = EEPROM.read(0);
    slot = findIDSLOT( a );
    start = (slot * 4) + 2;
    looping = ((num - slot) * 4);
    num--;      
    EEPROM.write( 0, num );  
    for ( j = 0; j < looping; j++ ) {        
      EEPROM.write( start + j, EEPROM.read(start + 4 + j));
    }
    for ( uint8_t k = 0; k < 4; k++ ) {
      EEPROM.write( start + j + k, 0);
    }
    successDelete();
    Serial.println(F("Succesfully removed ID record from memory"));
  }
}
bool checkTwo ( byte a[], byte b[] ) {   
  for ( uint8_t k = 0; k < 4; k++ ) {
    if ( a[k] != b[k] ) {
       return false;
    }
  }
  return true;  
}
uint8_t findIDSLOT( byte find[] ) {
  uint8_t count = EEPROM.read(0);   
  for ( uint8_t i = 1; i <= count; i++ ) { 
    readID(i);
    if ( checkTwo( find, storedCard ) ) { 
      return i;
    }
  }
}
bool findID( byte find[] ) {
  uint8_t count = EEPROM.read(0);
  for ( uint8_t i = 1; i < count; i++ ) {
    readID(i);      
    if ( checkTwo( find, storedCard ) ) { 
      return true;
    }
    else { 
    }
  }
  return false;
}
void successWrite() {
  digitalWrite(redLed, 0); 
  digitalWrite(greenLed, 1); 
  for(int x = 0; x < 3; ++x)
  {
    digitalWrite(buzzer, 1);
    delay(50);
    digitalWrite(buzzer, 0);
    delay(50);
  }
  delay(1000);
  digitalWrite(redLed, 0); 
  digitalWrite(greenLed, 0);
}
void failedWrite() {
  digitalWrite(redLed, 1); 
  digitalWrite(greenLed, 0); 
  for(int x = 0; x < 2; ++x)
  {
    digitalWrite(buzzer, 1);
    delay(200);
    digitalWrite(buzzer, 0);
    delay(100);
  }
  for(int x = 0; x < 2; ++x)
  {
    digitalWrite(redLed, 1);
    delay(100);
    digitalWrite(redLed, 0);
    delay(100);
  }
}
void successDelete() {
  digitalWrite(redLed, 0); 
  digitalWrite(greenLed, 1); 
  for(int x = 0; x < 3; ++x)
  {
    digitalWrite(buzzer, 1);
    delay(50);
    digitalWrite(buzzer, 0);
    delay(50);
  }
  delay(1000);
  digitalWrite(redLed, 0); 
  digitalWrite(greenLed, 0);
}
bool isMaster( byte test[] ) {
  return checkTwo(test, masterCard);
}

String compileId(byte arr[], int Size) {
  String id = "";
  for(int c = 0; c < Size; c++){
    id += String(arr[c], HEX);
  }
  return id;
}

int lookUp(String attendees[], int Size, String uid)
{
  for(int d = 0; d < Size / 6; ++d)
  {
    if(attendees[d] == uid)
    {
      return d + 1;
    }
  }
  return 0;
}
