/*
 * Reciever modul for the prototype for bridging my assistive system to IoT devices.
 * Works with the transmitter modul.
 * By Troels Madsen
 */
 
#include <SPI.h>
#include <WiFi.h>
#include <NTPClient.h> // Using a fork of the orginal NTPClient, which supports date output: https://github.com/taranais/NTPClient
#include <WiFiUdp.h>
#include <ArduinoHttpClient.h>
#include <Manchester.h>
#include "arduino_secrets.h"

/////// Manchester Settings //////
#define RX_PIN 4
#define LED_PIN 13
#define BUFFER_SIZE 16

/////// Wifi Settings ///////
char ssid[] = SECRET_SSID;
char pass[] = SECRET_PASS;

char server[] = "maker.ifttt.com"; // The IFTTT server
int port = 80;

String IFTTT_Key = SECTRET_IFTTT;
String IFTTT_Event = "alert";

WiFiClient wifi;
HttpClient client = HttpClient(wifi, server, port);
int status = WL_IDLE_STATUS;

String response;
int statusCode = 0;
uint8_t moo = 1;
uint8_t buffer[BUFFER_SIZE];

// The following is to fetch the time from NTP servers
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, 1);

void setup() {
  pinMode(LED_PIN, OUTPUT);
  
  //Initialize serial and wait for port to open:
  Serial.begin(9600);
  while (!Serial) {
    Serial.println("Waiting");
    // wait for serial port to connect. Needed for native USB port only.
    // Delete it in product, or headless setup.
  }
  
  Serial.println("******* Reciever setup for the prototype *********\nBy Troels Madsen\n\n");

  // check for the presence of the shield:
  if (WiFi.status() == WL_NO_SHIELD) {
    Serial.println("WiFi shield not present");
    // don't continue:
    while (true);
  }

  // attempt to connect to Wifi network:
  while (status != WL_CONNECTED) {
    Serial.print("Attempting to connect to network ");
    Serial.println(ssid);
    
    // Connect to WPA/WPA2 network:
    status = WiFi.begin(ssid, pass);
  }

  // Are connected now, print out the data:
  Serial.println("You're connected to the network");
  printWifiData();
  
  client.connectionKeepAlive(); // Let the connection stay alive
  digitalWrite(LED_PIN, moo); // Ligtens up the LED to tell that it's OK now.
  timeClient.begin(); // And start fetching the time

  // And in the last, begin to setup the FM receiver. It's better if WiFi is ready before FM.
  man.setupReceive(RX_PIN, MAN_1200);
  man.beginReceiveArray(BUFFER_SIZE, buffer);
  
  Serial.println("\nWe are ready, and are listening to a transmitter!\n");
}

void loop() {
  // Wait until a message is received by our receiver
  if ( man.receiveComplete() ) {
    uint8_t receivedSize = 0;

    uint8_t channel = buffer[0];
    uint8_t alert = buffer[1];
    
    Serial.print("Received data: ");
    Serial.print(channel);
    Serial.print(", ");
    Serial.println(alert);

    // Begin to send message using HTTP reques to the Webhook of IFTTT:
    send_IFTTT(channel, alert); // 1th var: channel, 2nd var: alert
    
    receivedSize = buffer[0];
    for(uint8_t i=1; i<receivedSize; i++)
      Serial.write(buffer[i]);

    Serial.println();
      
    man.beginReceiveArray(BUFFER_SIZE, buffer); // Start listening for the next message
    
    moo = ++moo % 2;
    digitalWrite(LED_PIN, moo);
      
    // Pause for 10 sec. Not necassary in this prototype, as the transmitter pauses the pulses.
    // delay(10000);
  }
}

void send_IFTTT(uint8_t channel, uint8_t alert) {
  timeClient.update(); // First update the timestamp by NTP
  
  Serial.println("Making POST request");
  String contentType = "application/json";  
  String path = "/trigger/"+IFTTT_Event+"/with/key/"+IFTTT_Key;

  Serial.println("Path: "+path);
  
  String date = timeClient.getFormattedDate();

  String jsonPayload = "{\"value1\":\""+String(channel)+"\",\"value2\":\""+String(alert)+"\",\"value3\":\""+date+"\"}";

  Serial.println("Payload: "+jsonPayload);

  client.post(path, contentType, jsonPayload);

  // read the status code and body of the response
  statusCode = client.responseStatusCode();
  response = client.responseBody();

  Serial.print("Status code: ");
  Serial.println(statusCode);
  Serial.print("Response: ");
  Serial.println(response);
}

void printWifiData() {
  // print the SSID of the network you're attached to:
  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());

  // print the MAC address of the router you're attached to:
  byte bssid[6];
  WiFi.BSSID(bssid);
  Serial.print("BSSID: ");
  Serial.print(bssid[5], HEX);
  Serial.print(":");
  Serial.print(bssid[4], HEX);
  Serial.print(":");
  Serial.print(bssid[3], HEX);
  Serial.print(":");
  Serial.print(bssid[2], HEX);
  Serial.print(":");
  Serial.print(bssid[1], HEX);
  Serial.print(":");
  Serial.println(bssid[0], HEX);

    // print your MAC address:
  byte mac[6];
  WiFi.macAddress(mac);
  Serial.print("MAC address: ");
  Serial.print(mac[5], HEX);
  Serial.print(":");
  Serial.print(mac[4], HEX);
  Serial.print(":");
  Serial.print(mac[3], HEX);
  Serial.print(":");
  Serial.print(mac[2], HEX);
  Serial.print(":");
  Serial.print(mac[1], HEX);
  Serial.print(":");
  Serial.println(mac[0], HEX);

  // print your WiFi shield's IP address:
  IPAddress ip = WiFi.localIP();
  Serial.print("IP Address: ");
  Serial.println(ip);

  // print the received signal strength:
  long rssi = WiFi.RSSI();
  Serial.print("signal strength (RSSI):");
  Serial.println(rssi);

  // print the encryption type:
  byte encryption = WiFi.encryptionType();
  Serial.print("Encryption Type:");
  Serial.println(encryption, HEX);
  
  Serial.println();
}
