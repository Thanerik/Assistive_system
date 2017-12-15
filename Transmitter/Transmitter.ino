
#include <Manchester.h>

/*
 * Transmitter modul for the prototype for bridging my assistive system to IoT devices.
 * Works with the receiver modul.
 * By Troels Madsen
 */

#define TX_PIN  5  //pin where your transmitter is connected
#define LED_PIN 13 //pin for blinking LED
#define data_size 16

uint8_t moo = 1; //last led status
uint8_t data[data_size] = {33, 33}; // Just assigned them a random number NOT between 0 and f...

boolean channel = false;
boolean alert = false;

void setup() {
  Serial.begin(9600);
  Serial.println("**** Transmitter setup for the prototype ****\nBy Troels Madsen\n\n");
    
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, moo);
  
  man.setupTransmit(TX_PIN, MAN_1200);
}

void loop() {
   Serial.println("Please choose a channel between 0 and 16 (A = 11, B = 12 .. F = 16).");
    while(!channel) {
      if( Serial.available() > 0 ) {
        int ch = Serial.parseInt();
        if(vertifyCh(ch)) {
          data[0] = ch;
          channel = true;
        }
        else {
          Serial.println("Try again, please!");
        }
      }
    }
    Serial.println("Thank you! Which alert? It must be between 0 and 7.");
    while(!alert) {
      if( Serial.available() > 0 ) {
        int al = Serial.parseInt();
        if(vertifyAl(al)) {
          data[1] = al;
          alert = true;
        }
        else {
          Serial.println("Try again, please!"); 
        }
      }
    }
    if(channel & alert) {
      Serial.println("Now we are set! Transmitting the data.");
      
      man.transmitArray(data_size, data);
      
      Serial.print("Data ");
      Serial.print(data[0]);
      Serial.print(", ");
      Serial.print(data[1]);
      Serial.println(" is sent. We are ready again about 10 seconds. Please wait.");
  
      moo = ++moo % 2;
      digitalWrite(LED_PIN, moo);
      
      // Wait 10 seconds
      eraseData();
      delay(10000);
      Serial.print("\n\n");
      Serial.println("We are ready again!");
    }
}

void eraseData() {
  alert = false;
  channel = false;
  data[0] = 33;
  data[1] = 33;
}

boolean vertifyCh(int data) {
    Serial.print("Vertifying Channel ");
    Serial.println(data);
    if(data == 33 || data < 0 || data > 16 ) {
      return false;
    }
  return true;
}
boolean vertifyAl(int data) {
    Serial.print("Vertifying Alarm ");
    Serial.println(data);
    if(data == 33 || data < 0 || data > 7 ) {
      return false;
    }
  return true;
}
