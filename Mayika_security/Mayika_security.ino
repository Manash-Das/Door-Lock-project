#include <Firebase_ESP_Client.h>
#include <addons/TokenHelper.h>
#include <addons/RTDBHelper.h>
#include <EEPROM.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#define API_KEY "AIzaSyBwjhRliABc0T_2n19gX6hNxKACs-rC4AE"
#define DATABASE_URL "https://esp-firebase-demo-ce5e6-default-rtdb.firebaseio.com/" //<databaseName>.firebaseio.com or <databaseName>.<region>.firebasedatabase.app
#define DATABASE_SECRET "DATABASE_SECRET"
const int LED = D6;
const int SWITCH_BTN = D1;
FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;
unsigned long dataMillis = 0;
String hotspot_ssid,hotspot_password,wifi_ssid,wifi_password,USER_EMAIL, USER_PASSWORD;
int hotspot_ssid_address = 0;
int hotspot_password_address = 30;
int wifi_ssid_address = 60;
int wifi_password_address = 90;
int user_email_address = 120;
int user_password_address = 150;
int button_state = LOW;
bool hotspot_on = false;
ESP8266WebServer server(80);
const char MAIN_page[] PROGMEM = R"=====(
<!DOCTYPE html>
<html lang="en">
<head>
    <style>
        body{background-attachment: fixed;background-image: linear-gradient(180deg,rgb(153, 152, 152),rgb(163, 156, 156));}
        .login {position: absolute;top:50%;left: 50%;transform: translateX(-50%) translateY(-50%);}
        th,td{font-size: 18px;font-family:Verdana, Geneva, Tahoma, sans-serif;}
        input{border: 1px solid;height: 25px;width: 200px;margin: 7px;padding-left: 10px;border-radius: 10px;background-color: rgb(255, 241, 241);}
        .Submit_button{font-size: 19px;width: 35%;}
        .login_text{text-align: center;}
    </style>
    <meta charset="UTF-8">
    <meta http-equiv="X-UA-Compatible" content="IE=edge">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Document</title>
</head>
<body><form class="login" action="/action_page">
        <table><tr><td colspan="2" class="login_text">LOGIN PAGE</td></tr>
              <tr><td><label>Email Id</label></td><td><input type="email" id="email" name="email_id"></td></tr>
              <tr><td><label>Password</label></td><td><input type = "password" id="password" name="email_password"></td></tr>
              <tr><td colspan="2"><center><input class= "Submit_button" type="submit" value="Submit"></center></td></tr></table></form></body></html>
)=====";

const char submitPage[] PROGMEM = R"=====(
<!DOCTYPE html>
<html lang="en">
<head>
    <style>
        .split {height: 100vh;position: fixed;top: 0%;}
        .left {right: 0%;width: 85%;background-color: #ffffff;}
        .right {width: 15%;left: 0%;background-color: #767171;}
        .centered {position: absolute;top: 50%;transform: translate(0,-50%);text-align: center;}
        .button{border: 1px solid;background-color: #bcb2b2;margin-bottom: 7px;width: 90%;padding: 5%;border-radius: 10px;font-size: 15px;}
        .center{position: absolute;top: 50%;left: 50%;transform: translateX(-50%) translateY(-50%);text-align: center;}
        input{margin: 10px;}
    </style>
    <meta charset="UTF-8">
    <meta http-equiv="X-UA-Compatible" content="IE=edge">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Document</title>
</head>
<body>
    <div class="split right">
        <div class="centered">
            <button class="button" id="lightControl">Light Control</button>
            <button class="button" id="changeWifi">Change Wifi</button>
            <button class="button" id="changeEmail">Change Email</button>
        </div>
    </div>
    <div class="split left">
        <div class="center" id="light"><h1>Welcome to control page</h1><form action="/light_btn"><button type="submit">Turn On</button></form></div>
        <div class="center" id="wifi"><h1>Change wifi setting</h1><form action="/submit_wifi">
                <input type="text" name="ssid" placeholder="ssid name"><br><input type="password" name="password" placeholder="ssid password"><br><button type="submit">save</button></form>
        </div>
        <div class="center" id="email"><h1>Change hotspot setting</h1>
            <form action="/submit_email"><input type="text" name="email" placeholder="mayikaofficial@gmail.com"><br><input type="password" name="password" placeholder="Mayika@official2023"><br><button type="submit">save</button></form>
        </div>
    </div>
    <script type="text/javascript">
        var x = document.getElementById("light");var y = document.getElementById("wifi");var z = document.getElementById("email");
        y.style.display = "none";z.style.display = "none";
    document.getElementById("lightControl").onclick = function(){x.style.display = "block";y.style.display = "none";z.style.display = "none";}
    document.getElementById("changeWifi").onclick = function(){x.style.display = "none";y.style.display = "block";z.style.display = "none";}
    document.getElementById("changeEmail").onclick = function(){x.style.display = "none";y.style.display = "none";z.style.display = "block";}
  </script>
</body>
</html>


)=====";

void setup() {
  // put your setup code here, to run once:
  EEPROM.begin(512);
  Serial.begin(9600);
  wifi_ssid = readStringFromEEPROM(wifi_ssid_address);
  wifi_password = readStringFromEEPROM(wifi_password_address);
  hotspot_ssid = readStringFromEEPROM(hotspot_ssid_address);
  hotspot_password = readStringFromEEPROM(hotspot_password_address);
  USER_EMAIL = readStringFromEEPROM(user_email_address);
  USER_PASSWORD = readStringFromEEPROM(user_password_address);

  pinMode(SWITCH_BTN,INPUT);
  pinMode(LED, OUTPUT);
  create_wifi();
}
void loop() {
  // put your main code here, to run repeatedly:
  button_state = digitalRead(SWITCH_BTN);
  if(!hotspot_on && button_state == HIGH){
    WiFi.mode(WIFI_OFF);
    createHotspot();
    hotspot_on = true;
  }
  else if(hotspot_on && button_state == LOW){
    WiFi.mode(WIFI_OFF);
    create_wifi();
    hotspot_on = false;
  }
  else if(!hotspot_on){
    if (millis() - dataMillis > 1000 && Firebase.ready()){
      dataMillis = millis();
      String path = "/UsersData/";
      path += auth.token.uid.c_str();
      path += "/Button";
      // Serial.println("Working");
      if (Firebase.RTDB.getInt(&fbdo, path)) {
        if (fbdo.dataType() == "string") {
          String intValue = fbdo.stringData();         
          if(intValue =="1"){
            digitalWrite(LED, HIGH);
            delay(10000);
            digitalWrite(LED,LOW);
            if(Firebase.RTDB.setString(&fbdo, path, "0")){
              Serial.println("Succesfully saved");
            }
            else{
              Serial.println("Not saved");
            }
          }
        }
        else{
          Serial.println("something went wrong");
        }
      }
      else {
        Serial.println(fbdo.errorReason());
          
      }
    }
  }
  server.handleClient();
  delay(3000);
}

void createHotspot(){
  server.on("/", homepage);
//  server.on("/action_page",handleform);
//  server.on("/submit_wifi",saveWifi);
//  server.on("/submit_email",saveEmail);
//  server.on("/light_btn",turnOn);
  server.onNotFound(handle_NotFound);
  server.begin();
  Serial.print("Setting soft-AP ... ");
  delay(100);
  boolean result = WiFi.softAP(hotspot_ssid, hotspot_password);
  if(result == true){
    Serial.println("Ready");
      Serial.println(WiFi.softAPIP());
      Serial.println("HTTP server started");
  }
  else{
    Serial.println("Failed!");
    Serial.println("Trying again in 3 second");
    delay(3000);
    createHotspot();
  }
}
void homepage(){
  String code = MAIN_page;
  server.send(200,"text/html",code);
}
void handleform(){
  String email = server.arg("email_id"); 
  String password = server.arg("email_password"); 
  Serial.println(email);
  Serial.println(USER_EMAIL);
  Serial.println(password);
  Serial.println(USER_PASSWORD);
  if(USER_EMAIL==email && USER_PASSWORD == password){
    server.on("/action_page",handleform);
    server.on("/submit_wifi",saveWifi);
    server.on("/submit_email",saveEmail);
    server.on("/light_btn",turnOn);
    Serial.println("Succesfull");
    String s = submitPage;
    server.send(200, "text/html", s); //Send web page
  }
  else{
    Serial.println("Unsuccessfull");
    String s = MAIN_page;
    server.send(200,"text/html", s);
  }
}
void saveWifi(){
  String wifi = server.arg("ssid");
  String password = server.arg("password");
  Serial.println(wifi);
  Serial.println(password);
  if(wifi.length()!=0){
    writeStringToEEPROM(wifi_ssid_address, wifi);
  }
  if(password.length()!=0){
    writeStringToEEPROM(wifi_password_address, password);
  }
  String s = submitPage;
  server.send(200, "text/html", s);
}
void saveEmail(){
  String changeEmail = server.arg("email");
  String password = server.arg("password");
  if(changeEmail.length()!=0){
    writeStringToEEPROM(user_email_address, changeEmail);
  }
  if(password.length()!=0){
    writeStringToEEPROM(user_password_address, password);
  }
  String s = submitPage;
  server.send(200, "text/html", s);
}
void handle_NotFound(){
  server.send(404, "text/plain", "Not found");
}
void turnOn(){
  String s = submitPage;
  server.send(200, "text/html", s);
  Serial.println("Turn On");
  digitalWrite(LED, HIGH);
  delay(3000);
  digitalWrite(LED, LOW);
  Serial.println("Turn Off");
}
void create_wifi(){
  WiFi.begin(wifi_ssid, wifi_password);
  Serial.println("Connecting to Wi-Fi");
  while (WiFi.status() != WL_CONNECTED){
    delay(300);
    button_state = digitalRead(SWITCH_BTN);
    if(button_state==HIGH){
      return;
    }
  }
  Serial.println("connected");
  config.api_key = API_KEY;
  auth.user.email = USER_EMAIL;
  auth.user.password = USER_PASSWORD;
  config.database_url = DATABASE_URL;
  Firebase.reconnectWiFi(true);
  fbdo.setResponseSize(4096);
  String base_path = "/UsersData/";
  Firebase.begin(&config, &auth);
}
void writeStringToEEPROM(int addrOffset, const String &strToWrite){
  byte len = strToWrite.length();
  EEPROM.write(addrOffset, len);
  for (int i = 0; i < len; i++){
    EEPROM.write(addrOffset + 1 + i, strToWrite[i]);
  }
  EEPROM.commit();
}
String readStringFromEEPROM(int addrOffset){
  int newStrLen = EEPROM.read(addrOffset);
  char data[newStrLen + 1];
  for (int i = 0; i < newStrLen; i++)
  {
    data[i] = EEPROM.read(addrOffset + 1 + i);
  }
  data[newStrLen] = '\0';
  return String(data);
}
