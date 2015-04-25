#include <Console.h> 
#include <Bridge.h> 
#include <OneWire.h> 
#include <Process.h> 
#include <ctype.h> 
bool shellrun = false; 
Process p;

//////////PH//////////
#include <AltSoftSerial.h>
#define rx 2
#define tx 3

AltSoftSerial myserial(rx, tx);

char ph_data[20];
char computerdata[20];
byte received_from_computer=0;
byte received_from_sensor=0;
byte arduino_only=0;
byte startup=0;
float ph=0;
byte string_received=0;
////////////////////

//////////Fermentation rate//////////
int bubblecounts = 0;
////////////////////

//////////Temperature communication pin//////////
OneWire ds(2);
const int ledPin = 12;
int ledState = LOW;
int dailynotifier = 0;
////////////////////

void setup() {
  delay(1000);
  pinMode(ledPin, OUTPUT);
  Bridge.begin();
  delay(1000);
  myserial.begin(38400); 
  delay(1000);
  attachInterrupt(0, bubble, CHANGE);
  delay(1000);
}

void loop() {
  //////////Gravity
  char buffer[5];
  String SpecificGravity = dtostrf((SGVoltage(A5)), 1, 4, buffer);
  SpecificGravity.trim();
  Bridge.put("SpecificGravity", SpecificGravity);
  delay(200);
  
  //////////Temperature
  String Temp = temperature();
  Bridge.put("Temperature",Temp);
  delay(200);
  
  //////////Fermentation
  String Fermrate = String(bubblecounts);
  Bridge.put("Fermrate",Fermrate);
  bubblecounts = 0; 
  delay(200);
  
  //////////pH
  String pHval = "0.00";
  float pH = 0.00;
  
  do {
    myserial.print("C\r");
    delay (1000);
    
    if(myserial.available() > 0){
      received_from_sensor=myserial.readBytesUntil(13,ph_data,20);
      ph_data[received_from_sensor]=0;
      string_received=1;
      pHval = String (ph_data);
      pH=atof(ph_data);
    }    
  } while ((pH<2.00) || (pH>9.00)||sizeof(pHval)<4) ;
      
   Bridge.put("pH",pHval);
    
  //////////Day
  dailynotifier++;
  Bridge.put("DAY",String(dailynotifier));
  delay(200);
  Bridge.put("ACK","SCRIPTGO");
  
  //////////Python
  p.begin("python");
  p.addParameter("/mnt/sda1/arduino/www/Pherm/cgi-bin/PhermMain.py"); 
  p.runAsynchronously();
  delay(10000);
  int cnt = 0;
  String ack;
 
  do {
    char myData[10];
    Bridge.get("ACK",myData,10);
    ack = String (myData);
    delay(500);
    cnt++;
    if (cnt >50 ){ ack = "ACK";}
  }while(ack.equals("SCRIPTGO"));
  
  Bridge.put("ACK","SCRIPTSTOP");
  
  if (dailynotifier>=24){dailynotifier = 0;}
  
  delay(600000); 
}

//////////Bubble//////////
void bubble() {
  bubblecounts++;
  
  if (ledState == LOW) {
    ledState = HIGH;
  }
  else {ledState = LOW;}
  
  digitalWrite(ledPin, ledState);
}

//////////phFactoryDefault//////////
void phFactoryDefault(){
  myserial.print("X\r");
}

//////////Handling Temperature//////////
String temperature(void) {
  byte i;
  byte data[12];
  byte addr[8];
  int Temp;
  
  do {
    delay(10);
  }while(!ds.search(addr));
  
  if ( OneWire::crc8( addr, 7) != addr[7]) {
    return "Invalid CRC";
  }
  
  if ( addr[0] != 0x28) {
    return "Wrong Device";
  }
  
  ds.reset();
  ds.select(addr);
  ds.write(0x44,1);
  delay(1000);
  
  ds.reset();
  ds.select(addr);
  ds.write(0xBE);
  
  for ( i = 0; i < 9; i++) {
    data[i] = ds.read();
  }
  
  Temp=(data[1]<<8)+data[0];
  Temp=Temp>>4;
  
  Temp=Temp*1.8+32;
  
  String str = String(Temp);
  return str;
}

//////////SG Volyage//////////
float SGVoltage(int channelnumber) {
  return (analogRead(channelnumber) * (5.0 / 1023.0));
}
