//Nessesary libraries for sensors
#include <SimpleDHT.h>
#include <EEPROM.h>
#define pinDHT11 2
#define MAXP 6
#define BUFFSIZE 4

SimpleDHT11 dht11;
byte temps[MAXP];
byte humid[MAXP];
int buff;
int tail;
int head;
int req;

void printTemps();

//
void setup() {
    Serial.begin(9600);
    //attachInterrupt(digitalPinToInterrupt(button), printTemps, CHANGE);
    //reads from EEPROM
    head=(EEPROM.read(MAXP<<1))*2*2*2*2*2*2*2*2+EEPROM.read(MAXP<<1|1);
    tail=(EEPROM.read((MAXP<<1)+2))*2*2*2*2*2*2*2*2+EEPROM.read((MAXP<<1)+3);
    debugPrintFromMemory();
    buff=0;
}

//main loop
void loop() {
    Serial.println("Starting Loop");
  //Writes temperature and humidity into RAM
    byte temperature = 0;
    byte humidity = 0;
    byte c=0;
    while(dht11.read(pinDHT11, &temperature, &humidity, NULL)&&c<51) {
        Serial.println("Read DHT11 failed.");  
        c++;
    }
    c=0;
    
    temps[tail] = temperature;
    humid[tail] = humidity;
  
    buff++;
    Serial.print("buff: ");
    Serial.println(buff); 
    //Doesn't write outside of memory by restricting MAXP such that MAXP*2+4<=EEPROM size
    
    //Writes tempurature and humidity into memory
    if(buff==BUFFSIZE){
      Serial.println("Starting Write To Memory");
       while(buff--){
          if(tail-buff>=0){
          EEPROM.update((tail-buff)<<1,temps[tail-buff]);
          EEPROM.update((tail-buff)<<1|1,humid[tail-buff]);
          }
          else{
          EEPROM.update((MAXP+tail-buff)<<1,temps[MAXP+tail-buff]);
          EEPROM.update((MAXP+tail-buff)<<1|1,humid[MAXP+tail-buff]);
          }
        }
        EEPROM.update(MAXP<<1,(byte)(head>>8));
        EEPROM.update(MAXP<<1|1,(byte)(head));
        EEPROM.update((MAXP<<1)+2,(byte)(tail>>8));
        EEPROM.update((MAXP<<1)+3,(byte)(tail));
        Serial.print("Unplug Now");
     }
    
    
    //Checks for a query, if it exists it retrieves n recent ones
    if(Serial.available()>0){
    req=Serial.read();
    }
    while(req--){
    if(tail-req>=0){
    Serial.write(temps[tail-req]);
    Serial.write(humid[tail-req]);
    }
    else{
    Serial.write(temps[tail-req+MAXP]);
    Serial.write(humid[tail-req+MAXP]);
    }
    }

    
    Serial.print(head);
    Serial.print(tail);
    
    //button interrupt
    delay(10000);
    tail=(tail+1)%MAXP;
    if(tail==head)head=(tail+1)%MAXP;
    printTemps();

}
void debugPrintFromMemory(){
    int t=EEPROM.read((MAXP<<1)+2)*2*2*2*2*2*2*2*2+EEPROM.read((MAXP<<1)+3);
    int r=EEPROM.read(MAXP<<1)*2*2*2*2*2*2*2*2+EEPROM.read(MAXP<<1|1);
    Serial.print(r);
    Serial.print(t);
    Serial.println();
    int point=head;
    do{
    Serial.print((int)EEPROM.read(point<<1));
    Serial.print("*C ");
    Serial.print((int)EEPROM.read(point<<1|1));
    Serial.print("%|");
    Serial.println();
    delay(1000);
    point=(point+1)%MAXP;
    }while(point!=tail);
}

void printTemps() {
    for(int i = head; i < tail; i++) {
        Serial.print((int)temps[i]);
        Serial.print("*C ");
        Serial.print((int)humid[i]);
        Serial.print("%|");
    }
    Serial.println();
}
