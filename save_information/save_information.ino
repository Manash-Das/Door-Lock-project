#include <EEPROM.h>
String USER_EMAIL = "mayikaofficial@gmail.com";
String USER_PASSWORD = "Mayika@official2023";
String hotspot_ssid = "Esp8266";
String hotspot_password = "123456789";
String wifi_ssid = "Mayika@2023";
String wifi_password = "123456789";
int hotspot_ssid_address = 0;
int hotspot_password_address = 30;
int wifi_ssid_address = 60;
int wifi_password_address = 90;
int user_email_address = 120;
int user_password_address = 150;
void setup() {
  EEPROM.begin(512);
  Serial.begin(9600);
  // put your setup code here, to run once:
    writeStringToEEPROM(hotspot_ssid_address, hotspot_ssid);
    writeStringToEEPROM(hotspot_password_address, hotspot_password);
  
    writeStringToEEPROM(wifi_ssid_address, wifi_ssid);
    writeStringToEEPROM(wifi_password_address, wifi_password);
   
   
    writeStringToEEPROM(user_email_address, USER_EMAIL);
    writeStringToEEPROM(user_password_address, USER_PASSWORD);
}

void loop() {
  // put your main code here, to run repeatedly:
  
  wifi_ssid = readStringFromEEPROM(wifi_ssid_address);
  wifi_password = readStringFromEEPROM(wifi_password_address);
  hotspot_ssid = readStringFromEEPROM(hotspot_ssid_address);
  hotspot_password = readStringFromEEPROM(hotspot_password_address);
  USER_EMAIL = readStringFromEEPROM(user_email_address);
  USER_PASSWORD = readStringFromEEPROM(user_password_address);
  Serial.println(wifi_ssid);
  Serial.println(wifi_password);
  Serial.println(hotspot_ssid);
  Serial.println(hotspot_password);
  Serial.println(USER_EMAIL);
  Serial.println(USER_PASSWORD);
}
void writeStringToEEPROM(int addrOffset, const String &strToWrite){
  int len = strToWrite.length();
  EEPROM.write(addrOffset, len);
  for (int i = 0; i < len; i++)
  {
    EEPROM.write(addrOffset + 1 + i, strToWrite[i]);
  }
  EEPROM.commit();
}
String readStringFromEEPROM(int addrOffset){
  int newStrLen = EEPROM.read(addrOffset);
  char data[newStrLen + 1];
  for (int i = 0; i < newStrLen; i++){
    data[i] = EEPROM.read(addrOffset + 1 + i);
  }
  data[newStrLen] = '\0';
  return String(data);
}
