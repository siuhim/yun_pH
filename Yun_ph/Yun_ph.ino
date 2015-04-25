#include <Bridge.h>

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


void setup(){
    Bridge.begin();
    myserial.begin(38400);
    Serial.begin(38400);
}

  
void loop(){ 
  String pHval = "0.00";
  float pH = 0.00;
 
    delay (1000);
    
    if(myserial.available() > 0){
      received_from_sensor=myserial.readBytesUntil(13,ph_data,20);
      ph_data[received_from_sensor]=0;
      string_received=1;
      pHval = String (ph_data);
      pH=atof(ph_data);
      myserial.print(pHval);
    }    
  
  Bridge.put("pH",pHval);
  myserial.print(pHval);
  
  
  Serial.println(pHval);
  
 }      
  
  


   


  

