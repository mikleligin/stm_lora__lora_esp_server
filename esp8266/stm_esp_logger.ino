#include <ESP8266HTTPClient.h>
#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <SoftwareSerial.h>
#include <string>
const char *SSID = "iPhone monke";
const char *PWD = "bbbbbbbb";
String key = "helloeveryoneiamj";
String chiper = "                 ";
String allStr = "";
String mac_adr = "";
WiFiClient wifiClient = WiFiClient();
long lastime = 0;

SoftwareSerial LoRa(14,12);



void web_send(String value)
{
    allStr = "mac_address=" + chiper;
    HTTPClient http;
    String link = "http://89.108.98.74/api/v1/"+value;
    http.begin(wifiClient, link);
    http.addHeader("Content-Type", "application/x-www-form-urlencoded");
    http.addHeader("Accept", "*/*");
    http.addHeader("Connection", "keep-alive");
    http.addHeader("User-Agent", "Mozilla/5.0 (Windows NT 10.0; rv:104.0) Gecko/20100101 Firefox/104.0");
    http.addHeader("Host", "http://89.108.98.74");
    Serial.println(allStr);
    int httpCode = http.POST(allStr);
    String payload = http.getString();
    Serial.print("Here callback");
    Serial.println(httpCode);   //Распечатать код возврата HTTP
    Serial.println(payload);
}

void connectToWiFi() {
  Serial.print("Connecting to ");
  Serial.println(SSID);
    WiFi.begin(SSID, PWD);
    while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
   // we can even make the ESP32 to sleep
 }
  Serial.print("Connected - ");
  Serial.println(WiFi.localIP());
  Serial.println(WiFi.macAddress());
  mac_adr = WiFi.macAddress();
  digitalWrite(LED_BUILTIN, HIGH);
  digitalWrite(LED_BUILTIN, LOW);
}
 
void setup() {

 Serial.begin(9600);
 connectToWiFi();
 LoRa.begin(9600);
 pinMode(LED_BUILTIN, OUTPUT);
 digitalWrite(LED_BUILTIN, HIGH);
 digitalWrite(LED_BUILTIN, LOW);
  for(int i = 0 ; i<17; i++)
  {
    chiper[i]=mac_adr[i]^key[i];
  }
  Serial.println(mac_adr);
  Serial.println(chiper);
}
String msg;
void blink()
{
  for(int i = 0; i<3; i++)
    {
      digitalWrite(LED_BUILTIN, HIGH);  // turn the LED on (HIGH is the voltage level)
      delay(100);                      // wait for a second
      digitalWrite(LED_BUILTIN, LOW);   // turn the LED off by making the voltage LOW
      delay(100); 
    }
  digitalWrite(LED_BUILTIN, HIGH);
}
void callback(String message) {
  // message.remove('\n');
  // Serial.println(message);
  if(message.indexOf("NEFT_GAS")>0)
  {
    blink();
    Serial.println("GAS");
    web_send("gaslogger");
  }
}

void loop() {
 //long now = millis();
 digitalWrite(LED_BUILTIN, LOW);
 digitalWrite(LED_BUILTIN, HIGH);
 digitalWrite(LED_BUILTIN, LOW);
 if(LoRa.available())
 {
    char c = LoRa.read();
      if(c =='\n')
      {
        //Serial.print(msg);
        callback(msg);
        msg = " ";
      }
      msg += c; 
 }
//  if(now - lastime > 1000) {
//    lastime = now;  
//  }
}