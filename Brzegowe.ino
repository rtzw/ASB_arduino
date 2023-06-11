#include <WiFi.h>
#include <PubSubClient.h>
#include <Wire.h>
#include <Adafruit_BME280.h>
#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <LiquidCrystal_I2C.h>
#include <MFRC522.h>
#include <SPI.h>
#include <math.h>
#include <ArduinoJson.h>

LiquidCrystal_I2C lcd(0x3f, 16, 2);
#define DHTPIN 13  
#define DHTTYPE DHT11


const char* ssid = ""; //ssid
const char* password = ""; //haslo

const char* mqtt_server = "broker.emqx.io";
String deviceId = "esp001";
WiFiClient espClient;
PubSubClient client(espClient);
long lastMsg = 0;
char msg[50];
int value = 0;

DHT dht(DHTPIN, DHTTYPE);
float temperature = 0;
float humidity = 0;
int globalTemperature = 25;
bool fanAuto = true;
bool fanStatus = false;
const char* url = "43aa-45-152-45-110.eu.ngrok.io";
const size_t bufSize = 700;
String fanStatusTopic = "ptcpapesvrwe/"+ deviceId+ "/fan/status";
String fanAutoStatusTopic = "ptcpapesvrwe/"+ deviceId+ "/fan/auto";
String maxTemperatureTopic = "ptcpapesvrwe/"+ deviceId+ "/max/temperature";
String newCardStatusTopic = "ptcpapesvrwe/"+ deviceId +"/newCard/status";
String getCardStatusTopic = "ptcpapesvrwe/"+ deviceId +"/newCard/get";
bool newCardStatus = false;
bool newCardLCDPrint = false;
DynamicJsonDocument doc(bufSize);
String rfidAdmin = "18924425307";
// LED Pin
const int ledPin = 4;
// RDIF
#define SS_PIN 2
#define RST_PIN 5
MFRC522 mfrc522(SS_PIN, RST_PIN); 

void setup() {
  SPI.begin();
  mfrc522.PCD_Init(); 
  Serial.begin(115200);
  lcd.begin();
  lcd.backlight(); 
  lcd.setCursor(0,0);
 
  dht.begin();
  Serial.println("Start!");
  lcd.print("Start");
  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
  pinMode(ledPin, OUTPUT);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }


if (!espClient.connect(url, 80)) {
    Serial.println("connection failed");
    return;
  }

  String resource = "/getEsp?espId=" + deviceId;

  espClient.print(String("GET ") + resource + " HTTP/1.1\r\n" +
               "Host: " + url + "\r\n" +
               "Connection: close\r\n\r\n");
  delay(500);

while (espClient.available()) {
  String line = espClient.readStringUntil('\n');
  if (line == "0\r") {
    break;
  }
  else {
    // Czytamy blok danych
    String jsonResponse = espClient.readStringUntil('\n');
    
    // Pomińmy końcową linie "\r" jeżeli istnieje
    if (jsonResponse.endsWith("\r")) {
      jsonResponse = jsonResponse.substring(0, jsonResponse.length() - 1);
    }
    

 Serial.println(jsonResponse);
  deserializeJson(doc, jsonResponse);
  

  globalTemperature = (int) doc["temperature"];
  fanAuto = doc["isFanAuto"];
  fanStatus = doc["isFanOn"];
  if(fanStatus == true) digitalWrite(ledPin, HIGH);


  }
}
}

void setup_wifi() {
  delay(10);
  // We start by connecting to a WiFi network
  Serial.print("Connecting to ");
  Serial.println(ssid);
}

void callback(char* topic, byte* message, unsigned int length) {
  Serial.print("Message arrived on topic: ");
  Serial.print(topic);
  Serial.print(". Message: ");
  String messageTemp;
  
  for (int i = 0; i < length; i++) {
    Serial.print((char)message[i]);
    messageTemp += (char)message[i];
  }
  Serial.println();

  if (String(topic) == fanStatusTopic) {
    if(messageTemp == "on"){
      Serial.println("fan on");
      fanStatus = true;
      digitalWrite(ledPin, HIGH);
    }
    else if(messageTemp == "off"){
      Serial.println("fan off");
      fanStatus = false;
      digitalWrite(ledPin, LOW);
    }
  }
  else if (String(topic) == fanAutoStatusTopic) {
    if(messageTemp == "on"){
      fanAuto = true;
    }
    else if(messageTemp == "off"){
      fanAuto = false;
    }
  }
  else if (String(topic) == maxTemperatureTopic) {
    globalTemperature = messageTemp.toInt();
  }
  else if(String(topic) == newCardStatusTopic){
        Serial.print(messageTemp);

      if(messageTemp == "on"){
      newCardStatus = true;
    }
    else if(messageTemp == "off"){
      newCardStatus = false;
    }
  }
}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    // Attempt to connect
    if (client.connect("ESP8266Client" + random(1000))) {
      Serial.println("MQTT connected");
      // Subscribe
      client.subscribe((fanStatusTopic).c_str());
      client.subscribe((fanAutoStatusTopic).c_str());
      client.subscribe((maxTemperatureTopic).c_str());
      client.subscribe((newCardStatusTopic).c_str());
    } else {
      Serial.print("MQTT error");
      Serial.print(client.state());

      delay(5000);
    }
  }
}
void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();
  delay(100);
  if(!newCardStatus){
  newCardLCDPrint = false;

  RfidScan();
  long now = millis();
  if (now - lastMsg > 5000) {
    lastMsg = now;
    // Temperature in Celsius
    temperature = dht.readTemperature(); 
    // Convert the value to a char array
    char tempString[8];
    dtostrf(temperature, 1, 2, tempString);
    Serial.print("Temperature: ");
    Serial.println(tempString);
    client.publish(("ptcpapesvrwe/"+deviceId+"/sensor/temperature").c_str(), tempString);
    
    lcd.setCursor(0,0);
    lcd.print("Temperat:"); 
    lcd.setCursor(10,0);
    lcd.print(tempString);
    humidity = dht.readHumidity();
    // Convert the value to a char array
    char humString[8];
    dtostrf(humidity, 1, 2, humString);
    Serial.print("Humidity: ");
    Serial.println(humString);
    client.publish(("ptcpapesvrwe/"+deviceId+"/sensor/humidity").c_str(), humString);
    lcd.setCursor(0,1);
    lcd.print("Humidity:"); 
    lcd.setCursor(10,1);
    lcd.print(humString);

    if((int)temperature >= globalTemperature && fanAuto && !fanStatus){
      fanStatus = true;
      digitalWrite(ledPin, HIGH);
      client.publish((fanStatusTopic).c_str(), "on");
    } 

    if((int)temperature < globalTemperature && fanAuto && fanStatus){
      fanStatus = false;
      digitalWrite(ledPin, LOW);
      String messageTmp = "off";
      client.publish((fanStatusTopic).c_str(), "off");
    }
  }
  }
  else{
    if(!newCardLCDPrint){
    lcd.clear();
    lcd.print("Apply the card!");
    newCardLCDPrint = true;
    }
    RfidScanNew();
  }
}

void dump_byte_array(byte *buffer, byte bufferSize) {
  lcd.clear();
  lcd.print("Checking...");
  String rfidId = "";
  bool authorized = false;
  for (byte i = 0; i < bufferSize; i++) {
    rfidId += buffer[i] < 0x10 ? " 0" : " ";
    rfidId += buffer[i];
  } 
  rfidId.replace(" ", "");
  if (!espClient.connect(url, 80)) {
    Serial.println("connection failed");
    return;
  }
  String resource = "/card/isCardAuthorized?espId=" + deviceId + "&code=" + rfidId;

  espClient.print(String("GET ") + resource + " HTTP/1.1\r\n" +
               "Host: " + url + "\r\n" +
               "Connection: close\r\n\r\n");
  delay(500);

while (espClient.available()) {
  String line = espClient.readStringUntil('\n');
  if (line == "0\r") {
    break;
  }
  else {

    String jsonResponse = espClient.readStringUntil('\n');
    if (jsonResponse.endsWith("\r")) {
      jsonResponse = jsonResponse.substring(0, jsonResponse.length() - 1);
    }
    
  deserializeJson(doc, jsonResponse);
  
  authorized = doc.as<bool>();
  }
  }
  lcd.clear();
  if(rfidId == rfidAdmin || authorized){
  lcd.print("Access granted!");
  delay(2000);
  lcd.clear();
  lcd.print("Set temperature");
  lcd.setCursor(0,1);
  lcd.print("-->");
  lcd.setCursor(13,1);
  lcd.print("<--");
  int lastN = 0;
  int newN = 1;
  long last = 0;
  while(true){
    long now = millis();
    newN = 18 + (int)round(analogRead(A5)/500.0);
    lcd.setCursor(7,1);
    lcd.print(String(newN));
    delay(100);
    
    if(lastN != newN){
      last = now;
      lastN = newN;
    }
    else if(now - last > 5000){
      lcd.clear();
      lcd.setCursor(0,0);
      globalTemperature = newN;
      lcd.print("Saved!");
      delay(3000);
      lcd.clear();
      client.publish((maxTemperatureTopic).c_str(), String(globalTemperature).c_str());
      break;
    }
  }
  }
  else{
    lcd.print("Access denied!");
    delay(4000);
    lcd.clear();
  }
}

void dump_byte_array_new(byte *buffer, byte bufferSize) {
  String rfidId = "";
  for (byte i = 0; i < bufferSize; i++) {
    rfidId += buffer[i] < 0x10 ? " 0" : " ";
    rfidId += buffer[i];
  } 
  rfidId.replace(" ", "");
  client.publish((getCardStatusTopic).c_str(), rfidId.c_str());

  lcd.clear();
  lcd.print("Cart added!");
  delay(4000);
  newCardStatus = false;
  lcd.clear();

}


void RfidScan(){
  if ( ! mfrc522.PICC_IsNewCardPresent()) return;
  if ( ! mfrc522.PICC_ReadCardSerial()) return;
  dump_byte_array(mfrc522.uid.uidByte, mfrc522.uid.size);
}

void RfidScanNew(){
  if ( ! mfrc522.PICC_IsNewCardPresent()) return;
  if ( ! mfrc522.PICC_ReadCardSerial()) return;
  dump_byte_array_new(mfrc522.uid.uidByte, mfrc522.uid.size);
}


