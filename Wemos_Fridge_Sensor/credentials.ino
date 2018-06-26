/**VARIOUS READ AND WRITES TO EEPROM**/

void saveCreds(){
  EEPROM.begin(512);
  int eeAddress = 0;
  
  creds customVar;
  if(ssid[0] != '\0'){
    strcpy(customVar.ssid, ssid);
  }else{
    strcpy(customVar.ssid, loaded.ssid);
  }
  if(password[0] != '\0'){
    strcpy(customVar.ssid_pwd, password);
  }else{
    strcpy(customVar.ssid_pwd, loaded.ssid_pwd);
  }
  if(www_username[0] != '\0'){
    strcpy(customVar.www_name, www_username);
  }else{
     strcpy(customVar.www_name, loaded.www_name);
  }
  if(www_password[0] != '\0'){
    strcpy(customVar.www_pwd, www_password);
  }else{
    strcpy(customVar.www_pwd, loaded.www_pwd);
  }

  Serial.println("Writing: ");
  Serial.println(customVar.ssid);
  Serial.println(customVar.ssid_pwd);
  Serial.println(customVar.www_name);
  Serial.println(customVar.www_pwd);
      
  EEPROM.put(eeAddress, customVar);
  
  EEPROM.commit();
  EEPROM.end();
  Serial.print("Written custom data type!");
}

creds loadCreds(){
  creds loaded;

  EEPROM.begin(512);
  int eeAddress = 0;

  EEPROM.get(eeAddress, loaded);
  
  EEPROM.commit();
  EEPROM.end();
  
  Serial.print("Read custom data type!");
  
  return loaded; 
}

void saveSendEmail(boolean sendEmail){
  EEPROM.begin(512);
  int eeAddress = sizeof(creds); 
  
  if(sendEmail){
    EEPROM.put(eeAddress, sendEmail);
  }else{
    sendEmail = false;
    EEPROM.put(eeAddress, sendEmail);
  }
  

  EEPROM.commit();
  EEPROM.end();
}
boolean loadSendEmail(){
  EEPROM.begin(512);
  int eeAddress = sizeof(creds); 
  boolean state;
  
  EEPROM.get(eeAddress, state);

  EEPROM.commit();
  EEPROM.end();

  return state;
}

