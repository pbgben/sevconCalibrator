#include <SPI.h>
#include <mcp2515.h>

volatile bool interrupt = false;
struct can_frame canMsg;
struct can_frame canMsg1;
MCP2515 mcp2515(10);

void irqHandler() {
    interrupt = true;
}

byte filterList[] = {0x478, 0x4f8, 0x4f9, 0x4fa, 0x4fb};
int filterSize = 5;
int incomingByte = 0;
bool cansend=false;
byte bvolt;


#include "Adafruit_Keypad.h"
#include <LiquidCrystal_I2C.h>

LiquidCrystal_I2C lcd(0x27,16,2);  // set the LCD address to 0x3F for a 16 chars and 2 line display


const byte ROWS = 5; // rows
const byte COLS = 4; // columns
//define the symbols on the buttons of the keypads
char keys[ROWS][COLS] = {
  {'A','B','#','*'},
  {'1','2','3','U'},
  {'4','5','6','D'},
  {'7','8','9','X'},
  {'L','0','R','E'}
};
byte rowPins[ROWS] = {7, 6, 5, 4, 3}; //connect to the row pinouts of the keypad
byte colPins[COLS] = {17, 16, 15, 14}; //connect to the column pinouts of the keypad

//initialize an instance of class NewKeypad
Adafruit_Keypad customKeypad = Adafruit_Keypad( makeKeymap(keys), rowPins, colPins, ROWS, COLS);

void setup() {
  canMsg1.can_id  = 0x4fc;
  canMsg1.can_dlc = 2;
  canMsg1.data[0] = 0x08;
  canMsg1.data[1] = 0xff;

  Serial.begin(115200);
  customKeypad.begin();
  lcd.init();
  //lcd.clear();         
  lcd.backlight();      // Make sure backlight is on 
  // Print a message on both lines of the LCD.
  lcd.setCursor(4,0);   //Set cursor to character 2 on line 0
  lcd.print("SEVCON");
  lcd.setCursor(2,1);   //Move cursor to character 2 on line 1
  lcd.print("CALIBRATOR");
  
  mcp2515.reset();
  mcp2515.setBitrate(CAN_100KBPS, MCP_8MHZ);
  mcp2515.setNormalMode();
  
  Serial.println("------- CAN Calibrator 1.4 ----------");
  delay(100);
  attachInterrupt(0, irqHandler, FALLING);
}

void loop() {
  // put your main code here, to run repeatedly:
  if (interrupt) {
        interrupt = false;
        Serial.println("DATA");
    }
    while (mcp2515.checkReceive()){
    if (mcp2515.readMessage(&canMsg) == MCP2515::ERROR_OK) {

      if(filter(canMsg.can_id)){
        //if(canMsg.can_id!=0x478){lcd.clear(); delay(100);}
      //if(canMsg.can_id!=0x478){
      //  canMsg1.data[1] = 0x00;
      //  mcp2515.sendMessage(&canMsg1);
      //}
      if(canMsg.can_id == 0x4F8) lcd.setCursor(0,0);
      if(canMsg.can_id == 0x4F9) lcd.setCursor(8,0);
      if(canMsg.can_id == 0x4FA) lcd.setCursor(0,1);
      if(canMsg.can_id == 0x4FB) lcd.setCursor(8,1);
      if(canMsg.can_dlc > 0){
      for (int i = 0; i<canMsg.can_dlc; i++)  {  // print the data
        if(canMsg.can_id==0x478){
          bvolt=canMsg.data[0];
        }else{
          //Serial.print(String((char)canMsg.data[i]));
          Serial.print(String((char)canMsg.data[i]));

          lcd.print(String((char)canMsg.data[i]));
        }}
        
      }
      }
    }}
  customKeypad.tick();
  if(customKeypad.justPressed(88)) { canMsg1.data[1] = 0xff; cansend=true; Serial.println("home"); lcd.clear();}
  if(customKeypad.justReleased(88)) { canMsg1.data[1] = 0x00; cansend=true;}
  if(customKeypad.justPressed(85)) { canMsg1.data[1] = 0x80; cansend=true; Serial.println("up"); lcd.clear();}
  if(customKeypad.justReleased(85)) { canMsg1.data[1] = 0x00; cansend=true;}
  if(customKeypad.justPressed(68)) { canMsg1.data[1] = 0x40; cansend=true; Serial.println("down"); lcd.clear();}
  if(customKeypad.justReleased(68)) { canMsg1.data[1] = 0x00; cansend=true;}
  if(customKeypad.justPressed(76)) { canMsg1.data[1] = 0x20; cansend=true; Serial.println("left"); lcd.clear();}
  if(customKeypad.justReleased(76)) { canMsg1.data[1] = 0x00; cansend=true;}
  if(customKeypad.justPressed(82)) { canMsg1.data[1] = 0x10; cansend=true; Serial.println("right"); lcd.clear();}
  if(customKeypad.justReleased(82)) { canMsg1.data[1] = 0x00; cansend=true;}
  if(customKeypad.justPressed(66)) { canMsg1.data[1] = 0x08; cansend=true; Serial.println("plus"); lcd.clear();}
  if(customKeypad.justReleased(66)) { canMsg1.data[1] = 0x00; cansend=true;}
  if(customKeypad.justPressed(65)) { canMsg1.data[1] = 0x04; cansend=true; Serial.println("minus"); lcd.clear();}
  if(customKeypad.justReleased(65)) { canMsg1.data[1] = 0x00; cansend=true;}
  if(cansend){
    mcp2515.sendMessage(&canMsg1);
    Serial.println("Sent");
    cansend=false;
  }
//delay(10);

}

boolean filter(byte check)
{
  for (int x = 0; x < filterSize; x++)
  {
    if (filterList[x] == check)
    {
      return true;
    }    
  } 
  return false;
}