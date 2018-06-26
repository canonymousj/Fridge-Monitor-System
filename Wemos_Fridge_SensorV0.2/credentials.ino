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


void saveSensorConfig(){
 EEPROM.begin(512);
  int eeAddress = sizeof(creds);
  
  sensorConfig customVar;
  
    if(local_numSen != 0){
      customVar.numSen = local_numSen;
    }else{
     customVar.numSen =  loadedConfig.numSen;
    }
    if(local_updateRate != 0){
      customVar.updateRate = local_updateRate;
    }else{
     customVar.updateRate =  loadedConfig.updateRate;
    }
    
  for(unsigned int i = 0; i < 6; i++){
    
    if(local_senSetName[i][0] != '\0'){
      strcpy(customVar.senSetName[i], local_senSetName[i]);
    }else{
      strcpy(customVar.senSetName[i], loadedConfig.senSetName[i]);
    }
    if(local_channelID[i] != 0){
      customVar.channelID[i] = local_channelID[i];
    }else{
     customVar.channelID[i] =  loadedConfig.channelID[i];
    }
    if(local_writekey[i][0] != '\0'){
      strcpy(customVar.writekey[i], local_writekey[i]);
    }else{
      strcpy(customVar.writekey[i], loadedConfig.writekey[i]);
    }
    if(local_readkey[i][0] != '\0'){
      strcpy(customVar.readkey[i], local_readkey[i]);
    }else{
      strcpy(customVar.readkey[i], loadedConfig.readkey[i]);
    }
    if(local_widgetID[i] != 0){
      customVar.widgetID[i] = local_widgetID[i];
    }else{
     customVar.widgetID[i] =  loadedConfig.widgetID[i];
    }
  }
  Serial.println("Writing: ");
  Serial.print(customVar.numSen);
  Serial.print(customVar.updateRate);
  for(unsigned int i = 0; i < 6; i++){

    Serial.print(customVar.senSetName[i]);
    Serial.print(customVar.channelID[i]);
    Serial.print(customVar.writekey[i]); 
    Serial.print(customVar.readkey[i]); 
    Serial.print(customVar.widgetID[i]);

    Serial.println();
  }
  EEPROM.put(eeAddress, customVar);
  
  EEPROM.commit();
  EEPROM.end();
  Serial.print("Written custom data type!");
}

sensorConfig loadSensorConfig(){
  sensorConfig loadedConfig;

  EEPROM.begin(512);
  int eeAddress = sizeof(creds);

  EEPROM.get(eeAddress, loadedConfig);
  
  EEPROM.commit();
  EEPROM.end();
  
  Serial.print("Read custom data type!");
  /*Serial.println("Reading: ");
  Serial.print(loadedConfig.numSen);
  Serial.print(loadedConfig.updateRate);
  for(unsigned int i = 0; i < 6; i++){

    Serial.print(loadedConfig.senSetName[i]);
    Serial.print(loadedConfig.channelID[i]);
    Serial.print(loadedConfig.writekey[i].c_str()); 
    Serial.print(loadedConfig.readkey[i].c_str()); 
    Serial.print(loadedConfig.widgetID[i]);

    Serial.println();
  }*/
  
  return loadedConfig; 
}

void saveSendEmail(boolean sendEmail){
  EEPROM.begin(512);
  int eeAddress = sizeof(creds); 
  eeAddress += sizeof(sensorConfig);
  
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
  eeAddress += sizeof(sensorConfig);
  boolean state;
  
  EEPROM.get(eeAddress, state);

  EEPROM.commit();
  EEPROM.end();

  return state;
}

void saveSendSMS(boolean sendSMS){
  EEPROM.begin(512);
  int eeAddress = sizeof(creds);
  eeAddress += sizeof(sensorConfig);
  eeAddress += sizeof(boolean); 
  
  if(sendSMS){
    EEPROM.put(eeAddress, sendSMS);
  }else{
    sendSMS = false;
    EEPROM.put(eeAddress, sendSMS);
  }
  
  EEPROM.commit();
  EEPROM.end();
}
boolean loadSendSMS(){
  EEPROM.begin(512);
  int eeAddress = sizeof(creds); 
  eeAddress += sizeof(sensorConfig);
  eeAddress += sizeof(boolean); 
  
  boolean state;
  
  EEPROM.get(eeAddress, state);

  EEPROM.commit();
  EEPROM.end();

  return state;
}

