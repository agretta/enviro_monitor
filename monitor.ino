//Nessesary libraries for sensors
#include <SimpleDHT.h>

int pinDHT11 = 22;
SimpleDHT11 dht11;
byte temps[2000];
byte humid[2000];
int tail;
int head;

void printTemps();

//
void setup() {
    Serial.begin(9600);
    //attachInterrupt(digitalPinToInterrupt(button), printTemps, CHANGE);
    tail = 0;
    head = 0;
}

//main loop
void loop() {
    byte temperature = 0;
    byte humidity = 0;
    if (dht11.read(pinDHT11, &temperature, &humidity, NULL)) {
        Serial.print("Read DHT11 failed.");
    }

    temps[tail] = temperature;
    humid[tail] = humidity;

    tail++;

    //button interrupt
    delay(10000);
    printTemps();
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
