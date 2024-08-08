//reads from p21, p22,h2,voltage
#include <WiFi.h>
#include "ThingSpeak.h"
#include <Firebase_ESP_Client.h>
#include "addons/TokenHelper.h"
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include "addons/RTDBHelper.h"
LiquidCrystal_I2C lcd(0x27, 16, 2);
const char* ssid = "bhupendra";   
const char* password = "aayein123"; 
WiFiClient  client;
#define API_KEY "AIzaSyBra-79seiDYiDe0HNiWusepfIFrV4T1sk"
#define DATABASE_URL "https://powertheft-47a45-default-rtdb.asia-southeast1.firebasedatabase.app/"
FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;
unsigned long myChannelNumber = 2581352;
const char * myWriteAPIKey = "TAZ3A9PURRCODWTF";

void setup() {
  Serial.begin(115200);
  pinMode(32,INPUT); //p21
  pinMode(34,INPUT); //p22
  pinMode(35,INPUT); //h2
  pinMode(33,INPUT); //voltage
  if(WiFi.status() != WL_CONNECTED){
    Serial.print("Attempting to connect");
    while(WiFi.status() != WL_CONNECTED){
      WiFi.begin(ssid, password); 
      delay(5000);     
    } 
    Serial.println("\nConnected.");
  }
  ThingSpeak.begin(client);
  config.api_key = API_KEY;

  config.database_url = DATABASE_URL;
    if (Firebase.signUp(&config, &auth, "", "")){
    Serial.println("ok");

  }
  else{
    Serial.printf("%s\n", config.signer.signupError.message.c_str());
  }


  config.token_status_callback = tokenStatusCallback; 
  
  Firebase.begin(&config, &auth);
  Firebase.reconnectWiFi(true);
  lcd.init();                       
  lcd.backlight();                  
  lcd.clear();  

}

float currents[3];
float voltage;
float power[3];
float maxc;
int maxv;
int unit[3];
void loop() {

  readCurrents();
  //readVoltage();
  voltage = 225;
  sendTofb(voltage,"voltage/voltage2");
  cpower();

  sendtoTs(power[0]+power[1],2);
  delay(1000);
  sendtoTs(power[2],4);
  delay(500);
  sendTofb(power[0],"power/pole21");
  delay(1000);
  sendTofb(power[1],"power/pole22");
  delay(1000);
  sendTofb(power[1]+power[0],"power/pole2");
  sendTofb(power[2],"power/home2");
  updatelcd();
  sendTofb(unit[0],"units/pole21");
  sendTofb(unit[1],"units/pole22");
  sendTofb(unit[1]+unit[0],"units/pole2");
  sendTofb(unit[2],"units/home2");
  delay(2000);
}
float val;
void readCurrents(){
  maxc = 0;
  for(int i = 0;i<5000;i++){
    val = analogRead(32);
    if(val>maxc){
      maxc = val;
    }
  }
  maxc = maxc - 3150;
  maxc = maxc/190;
  maxc = 0.177*maxc;
  currents[0] = fabsf(maxc);
    maxc = 0;
  for(int i = 0;i<5000;i++){
    val = analogRead(34);
    if(val>maxc){
      maxc = val;
    }
  }
  maxc = maxc - 3150;
  maxc = maxc/190;
  maxc = 0.177*maxc;
  currents[1] = fabsf(maxc);
    maxc = 0;
  for(int i = 0;i<5000;i++){
    val = analogRead(35);
    if(val>maxc){
      maxc = val;
    }
  }
  maxc = maxc - 3150;
  maxc = maxc/190;
  maxc = 0.177*maxc;
  currents[2] = fabsf(maxc);
}
void readVoltage(){
  maxv = 0;
  for(int i = 0;i<100;i++){
    int val = analogRead(33);
    if(val>maxv){
      maxv = val;
    }
  }
  voltage = maxv;
}
void cpower(){
  power[0] = voltage*currents[0];
  power[1] = voltage*currents[1];
  power[2] = voltage*currents[2];
  unit[0] = unit[0] + power[0]/3600000;
  unit[1] = unit[1] + power[1]/3600000;
  unit[2] = unit[2] + power[2]/3600000;
}
void sendtoTs(int x,int y){
  int response = ThingSpeak.writeField(myChannelNumber, y, x, myWriteAPIKey);
  if(response == 200){
      Serial.println("Channel update successful.");
    }
    else{
      Serial.println("Problem updating channel. HTTP error code " + String(x));
    }
}
void sendTofb(float x,String y){
  if (Firebase.RTDB.setInt(&fbdo, y, x)){
      Serial.println("PASSED");
      Serial.println("PATH: " + fbdo.dataPath());
      Serial.println("TYPE: " + fbdo.dataType());
    }
    else {
      Serial.println("FAILED");
      Serial.println("REASON: " + fbdo.errorReason());
    }
}
void updatelcd(){
  String one = String(voltage) + " V" + String(currents[2]) + "A";
  String two = String(power[2]) + " W " + String(unit[2]) + " KWHr";
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print(one);
  lcd.setCursor(0,1);
  lcd.print(two);
}
