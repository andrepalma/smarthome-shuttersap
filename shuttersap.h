#include <EEPROM.h>
#define MEM_ALOC_SIZE 30

// start reading from the first byte (address 0) of the EEPROM
int address = 0;
byte valueFromEEP;

const int EEP_OFFSET_LEVEL = 0;
const int EEP_OFFSET_UP = 5;
const int EEP_OFFSET_DOWN = 10;


const int PIN_REL_UP = gv_PIN_REL_UP;
const int PIN_REL_DN = gv_PIN_REL_DN;

boolean ledValUp = false;    // state of LED 1
boolean ledValdn = false;    // state of LED 2
boolean relValUp = 0;    // state of LED 1
boolean relValDn = 0;    // state of LED 2


long timeForUp = 0;
long timeForDown = 0;
int levelOnEEP = 0;

long ratioUp = 0;
long ratioDown = 0;

int shutLevelToGo = 0;
int shutCurrLevel = 0;  // 0 - 100; 0 - Open, 100 Close
char chrShutCurrLevel[10] = "";

long shutCurrMili = 0;


enum ShuttersOperation {
  GoToLevel,
  GoingUP,
  GoingDOWN,
  HALT,
  Open,
  Closed
};

ShuttersOperation shutState = HALT;



void convertCurrLevelToChar(){
  String temp_str2;
  temp_str2 = String(shutCurrLevel);
  temp_str2.toCharArray(chrShutCurrLevel, temp_str2.length() + 1);
}

// int inChar = Serial.read();
// if (isDigit(inChar)) {
//   // convert the incoming byte to a char and add it to the string:
//   inString += (char)inChar;
// }
// if you get a newline, print the string, then the string's value:
// if (inChar == '\n') {
//   int test = inString.toInt();
//   Serial.print("Value: ");
//   Serial.println(test);
//   Serial.print("String: ");
//   Serial.println(inString);
//   // clear the string for new input:
//   inString = "";
//   Serial.print("One: ");
//   Serial.println((test%10));
//   Serial.print("Tens: ");
//   Serial.println(((test/10)%10));
//   Serial.print("hundreds: ");
//   Serial.println(((test/100)%10));
//   Serial.print("thousands: ");
//   Serial.println((test/1000));

// ones = (input_number%10);
// tens = ((input_number/10)%10);
// hundreds = ((input_number/100)%10);
// thousands = (input_number/1000);

void readeep(){
  if (address > 50) return;
  valueFromEEP = EEPROM.read(address);
  Serial.print(address);
  Serial.print("\t");
  Serial.print(valueFromEEP, DEC);
  Serial.println();
  address = address + 1;
  if (address == EEPROM.length()) {
    address = 0;
  }
}

void EEPROMWritelong(int addressOffSet, long value){
  EEPROM.begin(MEM_ALOC_SIZE);
  //Decomposition from a long to 4 bytes by using bitshift.
  //One = Most significant -> Four = Least significant byte
  byte four = (value & 0xFF);
  byte three = ((value >> 8) & 0xFF);
  byte two = ((value >> 16) & 0xFF);
  byte one = ((value >> 24) & 0xFF);

  EEPROM.write(addressOffSet, four);
  EEPROM.write(addressOffSet + 1, three);
  EEPROM.write(addressOffSet + 2, two);
  EEPROM.write(addressOffSet + 3, one);
  // Serial.printf("Bytes Write, Four: %d\t Three: %d\t Two: %d\t One: %d\n", (int)four, (int)three, (int)two, (int)one);
  EEPROM.end();
}

void EEPROMWriteint (int addressOffSet, int value){
  EEPROM.begin(MEM_ALOC_SIZE);
  EEPROM.write(addressOffSet, value);
  EEPROM.end();
}

long EEPROMReadlong(long address3){
  //Read the 4 bytes from the eeprom memory.
  long long_value;

  EEPROM.begin(MEM_ALOC_SIZE);
  byte four = EEPROM.read(address3);
  byte three = EEPROM.read(address3 + 1);
  byte two = EEPROM.read(address3 + 2);
  byte one = EEPROM.read(address3 + 3);
  EEPROM.end();

  long_value  = (long)one << 24;
  long_value += (long)two << 16;
  long_value += (long)three << 8;
  long_value += (long)four;

  return long_value;
}

int EEPROMReadInt(long addressOffSet){
  EEPROM.begin(MEM_ALOC_SIZE);
  uint8_t readvalue = EEPROM.read(addressOffSet);
  int ret = (int) readvalue;
  EEPROM.end();
  Serial.printf("IntLevel: %d\n", ret);
  return ret;
}

void writeLevelToEEPROM(){
  EEPROMWriteint(EEP_OFFSET_LEVEL, shutCurrLevel);
}

void writeTimeUpToEEPROM(long TimeGoUp){
  EEPROMWritelong(EEP_OFFSET_UP, TimeGoUp);
  timeForUp = TimeGoUp;
}

void writeTimeDnToEEPROM(long TimeGoDn){
  EEPROMWritelong(EEP_OFFSET_DOWN, TimeGoDn);
  timeForDown = TimeGoDn;
}

void calibrationRatio(){
  if (timeForUp > 100){
    ratioUp = timeForUp / 100;

  }
  if (timeForDown > 100){
    ratioDown = timeForDown / 100;
  }
  Serial.printf("RatioUp: %d, RationDown: %d\n", ratioUp, ratioDown);

}

void sendTimeForUp(){
  String temp_str2;
  char temp2[5];
  temp_str2 = "TimeForUp=" + String(timeForUp);
  temp_str2.toCharArray(temp2, temp_str2.length() + 1);
  // client.publish(gv_shuttersTopicStatPower,convertIntToChar(shutCurrLevel));
  // String messagePublish = "TimeForUp=" + temp2
  Serial.printf("Msg to be send: %s",temp2);
  client.publish(gv_shuttersTopicStatSettings,temp2);

}

void sendTimeForDn(){
  String temp_str2;
  char temp2[5];
  temp_str2 = "TimeForDn=" + String(timeForDown);
  temp_str2.toCharArray(temp2, temp_str2.length() + 1);
  // client.publish(gv_shuttersTopicStatPower,convertIntToChar(shutCurrLevel));
  // String messagePublish = "TimeForUp=" + temp2
  Serial.printf("Msg to be send: %s",temp2);
  client.publish(gv_shuttersTopicStatSettings,temp2);
}

void setTimeForUp(long newTime){
  timeForUp = newTime;
  writeTimeUpToEEPROM(timeForUp);
  calibrationRatio();
  sendTimeForUp();

}

void setTimeForDn(long newTime){
  timeForDown = newTime;
  writeTimeDnToEEPROM(timeForDown);
  calibrationRatio();
  sendTimeForDn();
}


void shutGoUp(){
  shutCurrMili = millis();
  digitalWrite(PIN_REL_UP, relValUp);
  digitalWrite(PIN_REL_DN, !relValDn);
  shutState = GoingUP;
  Serial.println("shutter Going UP.");

}

void shutGoDn(){
  shutCurrMili = millis();
  digitalWrite(PIN_REL_UP, !relValUp);
  digitalWrite(PIN_REL_DN, relValDn);
  shutState = GoingDOWN;
  Serial.println("shutter Going Down.");
}

void shutHalt(){
  digitalWrite(PIN_REL_UP, !relValUp);
  digitalWrite(PIN_REL_DN, !relValDn);
  shutState = HALT;
  Serial.println("shutter Stopped.");
}

void shutGoLevel(){



}

void cmdGoUp(){

  switch (shutState) {
    case GoingUP:
      shutHalt();
      // delay(20);
      Serial.println("Already going up, so stopped.");
      break;
    case GoingDOWN:
      shutHalt();
      // delay(20);
      Serial.println("Going up, so stopped.");
      // shutGoUp();
      break;
    case GoToLevel:
      shutHalt();
      Serial.println("GotoLevel, Halt");
      // delay(20);
      break;
    case HALT:
      shutGoUp();
      Serial.println("Halt, GoUp");
      // delay(20);
      break;
    case Open:
      shutHalt();
      Serial.println("Open, Halt");
      break;
    case Closed:
      Serial.println("Closed, GoUp");
      shutGoUp();
      break;
  }
}

void cmdGoDn(){
  switch (shutState) {
    case GoingUP:
      shutHalt();
      // delay(20);
      Serial.println("going Up, so stopped.");
      break;
    case GoingDOWN:
      shutHalt();
      // delay(20);
      Serial.println("Already Going Down, so stopped.");
      // shutGoUp();
      break;
    case GoToLevel:
      shutHalt();
      // delay(20);
      break;
    case HALT:
      shutGoDn();
      // delay(20);
      break;
    case Open:
      shutGoDn();
      break;
    case Closed:
      shutHalt();


      break;
  }
}

void cmdHalt(){
  shutHalt();
}

void cmdGoLevel(int levelReceived){
  shutLevelToGo = levelReceived;

}

int shutCheckState(){

  switch (shutState) {
    case GoToLevel:
      if (shutCurrLevel < shutLevelToGo){
        shutGoUp();
      }
      else if (shutCurrLevel > shutLevelToGo){
        shutGoDn();
      }
      else {
        shutHalt();
      }
      break;
    case GoingUP:
      if (shutCurrLevel == 100){
        shutHalt();
        // shutState = Open;
        // char* strteste = "open";
        client.publish(gv_shuttersTopicStatPower, "open");
        // client.publish(gv_shuttersTopicStatPower, strteste);
        Serial.println("Going UP stopped, level 100..");
      }
      else {
        if ((millis() - shutCurrMili) >= ratioUp) {
          shutCurrLevel++;
          convertCurrLevelToChar();
          client.publish(gv_shuttersTopicStatPower,chrShutCurrLevel);
          Serial.println("Going UP");
          writeLevelToEEPROM();
          shutCurrMili = millis();
        }
      }
      break;

    case GoingDOWN:
      if (shutCurrLevel == 0){
        shutHalt();
        // shutState = Closed;
        client.publish(gv_shuttersTopicStatPower, "closed");
        Serial.println("Going Down stopped, level 0..");
      }
      else {
        if ((millis() - shutCurrMili) >= ratioDown) {
          shutCurrLevel--;
          long curDif = millis() - shutCurrMili;

          convertCurrLevelToChar();
          String prt = "";
          prt += "Going down, CurrLevel: ";
          prt += chrShutCurrLevel;
          prt += ", ratio: ";
          prt += ratioDown;
          prt += ", currDif: ";
          prt += curDif;
          Serial.println(prt);

          client.publish(gv_shuttersTopicStatPower,chrShutCurrLevel);
          Serial.println("Going Down");
          writeLevelToEEPROM();
          shutCurrMili = millis();
        }
      }
      break;

    case HALT:

      break;
    case Open:

      break;
    case Closed:

      break;

  }
  // delay(10);
}

void shuttersSetup(){
  // writeTimeUpToEEPROM(17000);
  // writeTimeDnToEEPROM(17000);

  timeForUp = EEPROMReadlong(EEP_OFFSET_UP);
  timeForDown = EEPROMReadlong(EEP_OFFSET_DOWN);
  levelOnEEP = EEPROMReadInt(EEP_OFFSET_LEVEL);

  shutCurrLevel = levelOnEEP;
  convertCurrLevelToChar();
  calibrationRatio();
  pinMode(PIN_REL_UP, OUTPUT);
  digitalWrite(PIN_REL_UP, !relValUp);
  pinMode(PIN_REL_DN, OUTPUT);
  digitalWrite(PIN_REL_DN, !relValDn);

}

void shuttersLoop(){
  shutCheckState();

}
