//Nessesary libraries for sensors
#include <SimpleDHT.h>
#include <SoftwareSerial.h>
#include <EEPROM.h>
#define pinDHT11 2
#define PERIOD 3000
//Note: changing MAXP may cause data loss
#define MAXP 20
#define BUFFSIZE 3000000

SimpleDHT11 dht11;
byte temps[MAXP];
byte humid[MAXP];
int buff;
int tail;
int head;
int req=0;
int last=0;
SoftwareSerial btooth(11,10);

void setup() {
    btooth.begin(9600);
    Serial.begin(9600);
    //reads from EEPROM
    loadFromMemory();
    buff=0;
}

//main loop
void loop() {
  //Writes temperature and humidity into RAM
    byte temperature = 0;
    byte humidity = 0;
    byte c=0;
    while(dht11.read(pinDHT11, &temperature, &humidity, NULL)&&c<51) {
        c++;
    }
    c=0;

    
    temps[tail] = temperature;
    humid[tail] = humidity;
    buff++;
    // Serial.print("buff: ");
    //Serial.println(buff); 
    //Doesn't write outside of memory by restricting MAXP such that MAXP*2+6<=EEPROM size
    
    //Writes tempurature and humidity into memory
    if(buff==BUFFSIZE){
      //Serial.println("Starting Write To Memory");
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
        EEPROM.update((MAXP<<1)+4,(byte)(last>>8));
        EEPROM.update((MAXP<<1)+5,(byte)(last));
     }
    
    
    //Checks for a query, if it exists it retrieves n recent ones
    btooth.listen();

    readCommand();

    delay(PERIOD);
    tail=(tail+1)%MAXP;
}

/*
 * Reads, parses, and executes commands from the bluetooth connection
 * Each has the following format:
 * [type char][some bytes]
 * The type char distinguishes the type of command, while the bytes provide parameters for the command.
 * When the command sends back data, it prefaces the data with an error number (note that command-specific errors are checked in 
 * increasing order.)
 * 0 = no error.
 * 10 = unable to parse command.
 * Sent data always has the format [most historic temp][most historic humidity][next most historic temp]... unless otherwise specified
 * The other error numbers are command specific.
 * Commands
 * [u][byte 1][byte 2]: Sends the [[byte 1][byte 2]] most recently read values. 
 * 1 = user requested more data than has been read.
 * 2 = user requested more data than the buffer could possibly hold.
 * [r][byte 1][byte 2][byte 3][byte 4] : Sends the values in the range [s,f] (starting with s and then ending on f) inclusive, 
 * s=[[byte 1][byte 2]],f=[[byte 1][byte 2]] where sand f are the number of values between the most historic recorded value
 * and themselves.
 * 3 = user sent range that doesn't exist (e.g. f<s).
 * 4 = user sent range that is greater than what has been read.
 * 5 = user sent range that is greater than what could possibly be read.
 * [a] : Sends all the values that have been read
 * [p] : Returns the reading period in ms, most significant bit first [firstByte][secondByte]
 * [c] : Returns configuration (every sensor name is two bits)
 * [l] : Returns all data that hasn't already been sent. This results in undefined behavior if no data has been sent yet or if the buffer has overwritten the last sent value.
 */

void sendData(int err, int s, int f){
  btooth.write((byte)err);
  Serial.println("Error is: "+err);
  while(true){
    btooth.write(temps[s]);
    btooth.write(humid[s]);
    if(s==f){last=s;break;}
    s=(s+1)%MAXP;
  }
}


int readCommand(){
int err;
if(btooth.available()>0){
  char c=btooth.read();
  Serial.print("Command type:");
  Serial.println(c);
  if(c=='u')err=updateValues();
  else if(c=='r')err=rangeValues();
  else if(c=='a')err=all();
  else if(c=='p')err=sendPeriod();
//  else if(c=='c')err=sendConfig();
  else if(c=='l')err=sendLast();
  else err=10;
  if(err==10){
    btooth.write(10);
    return -1;
  }
  return 1;
}
return 0;
}
int sendLast(){
  sendData(0,last,tail);
  return 0;
}


int sendPeriod(){
  btooth.write((byte)0);
  int a=PERIOD;
  byte secondbyte=a;
  a>>=8;
  byte firstbyte=a;
  btooth.write(firstbyte);
  Serial.println(firstbyte);
  btooth.write(secondbyte);
  Serial.println(secondbyte);
  return 0;
}

int updateValues(){
  if(btooth.available()>1){
    Serial.println("Updating Values");
    int a=btooth.read();
    int b=btooth.read();
    Serial.println(a);
    Serial.println(b);
    int dist=tail-head+1;
    if(dist<=0)dist=MAXP;
    int count=(a<<8)+b;
    Serial.print("Count:");
    Serial.println(count);
    if(count>MAXP)btooth.write((byte)2);
    else if(count>dist)btooth.write((byte)1);
    else{
      btooth.write((byte)0);
      Serial.print("Writing Error:");
      Serial.println((byte)0);
      while(count--){
            if(tail-count>=0){
              btooth.write(temps[tail-count]);
              btooth.write(humid[tail-count]);
            }
            else{
              btooth.write(temps[tail-count+MAXP]);
              btooth.write(humid[tail-count+MAXP]);
            }
          }
    }
    return 0;
  }
  else return 10;
}

int rangeValues(){
if(btooth.available()>3){
  int a=btooth.read();
  int b=btooth.read();
  int s=(a<<8)+b;
  a=btooth.read();
  b=btooth.read();
  int f=(a<<8)+b;
  int dist=tail-head+1;
  if(dist<=0)dist=MAXP;
  if(f<s)btooth.write((byte)3);
  else if(f>=dist)btooth.write((byte)4);
  else if(s<0||f<0)btooth.write((byte)5);
  else{
    s=(s+head)%MAXP;
    f=(f+head)%MAXP;
    sendData(0,s,f);
  }
  return 0;
}
else return 10;
}

int all(){
btooth.write((byte)0);
int h=head;
int t=tail;
sendData(0,h,t);
return 0;
}


void loadFromMemory(){
    head=(EEPROM.read(MAXP<<1))*2*2*2*2*2*2*2*2+EEPROM.read(MAXP<<1|1);
    tail=(EEPROM.read((MAXP<<1)+2))*2*2*2*2*2*2*2*2+EEPROM.read((MAXP<<1)+3);
    last=(EEPROM.read((MAXP<<1)+4))*2*2*2*2*2*2*2*2+EEPROM.read((MAXP<<1)+5);
    int point=(head-1);
    if(point==-1)point=MAXP-1;
    do{
    point=(point+1)%MAXP;
    temps[point]=EEPROM.read(point<<1);
    humid[point]=Serial.print((int)EEPROM.read(point<<1|1));
    }while(point!=tail);
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

