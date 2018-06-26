#include "Gsender.h"

/**Handles root inline when in STA mode**/
void handleRoot() {
  if(!authed){  //server authentication message
    if(!server.authenticate(loaded.www_name, loaded.www_pwd))
        return server.requestAuthentication(DIGEST_AUTH, www_realm, authFailResponse);

    authed = 1;
    prevMillis = millis();  
  }     
  
  char temp[2500];
  int sec = millis() / 1000;
  int min = sec / 60;
  int hr = min / 60;

  snprintf ( temp,2500,
"<html>\
  <head>\
    <meta http-equiv='refresh' content='60'/>\
    <title>ESP8266 Fridge Sensor Controls</title>\
    <style>\
      body { background-color: #cccccc; font-family: Arial, Helvetica, Sans-Serif; Color: #000088; }\
    </style>\
  </head>\
  <body>\
    <h1>Sensor Controls</h1>\
    <p>Uptime: %02d:%02d:%02d</p>\
    <p>Email send state: %s</p>\
 <a href=\"/togemail\"><button>Toggle Email</button></a>\
  <p>SMS send state: %s</p>\
  <a href=\"/togsms\"><button>Toggle SMS</button></a>\
  <p>View sensors:</p>\
  <a href=\"/set1\"><button>View %s</button></a>\
  <a href=\"/set2\"><button>View %s</button></a>\
  <a href=\"/set3\"><button>View %s</button></a><br>\
  <a href=\"/set4\"><button>View %s</button></a>\
  <a href=\"/set5\"><button>View %s</button></a>\
  <a href=\"/set6\"><button>View %s</button></a>\
  <p>Configuration:</p>\
  <a href=\"/config\"><button>Configure system</button></a>\
  </body>\
</html>",
    hr, min % 60, sec % 60, (sendEmail?"True":"False"), (sendSMS?"True":"False"), loadedConfig.senSetName[0], loadedConfig.senSetName[1], loadedConfig.senSetName[2], loadedConfig.senSetName[3], loadedConfig.senSetName[4], loadedConfig.senSetName[5] 
  );
  
   server.send ( 200, "text/html", temp );
  
}

/**Handles root inline when in AP mode**/
void handleRootAP() {
  Serial.println("Handling root AP");
  server.sendHeader("Cache-Control", "no-cache, no-store, must-revalidate");
  server.sendHeader("Pragma", "no-cache");
  server.sendHeader("Expires", "-1");
  
  server.setContentLength(CONTENT_LENGTH_UNKNOWN);
  
  server.send(200, "text/html", ""); // Empty content inhibits Content-length header so we have to close the socket ourselves.
  
  server.sendContent(
    "<html><head></head><body>"
    "<h1>Wifi config</h1>"
  );
  
  server.sendContent(String("<p>You are connected through the soft AP: ") + toStringIp(WiFi.softAPIP()) + "</p>");
  
  server.sendContent(
    "\r\n<br />"
    "<table><tr><th align='left'>SoftAP config</th></tr>"
  );
  
  server.sendContent(String() + "<tr><td>SSID " + "FridgeInit" + "</td></tr>");
  server.sendContent(String() + "<tr><td>IP " + toStringIp(WiFi.softAPIP()) + "</td></tr>");
 
  server.sendContent(
    "</table>"
    "\r\n<br />"
    "<table><tr><th align='left'>WLAN list (refresh if any missing)</th></tr>"
  );
  
  Serial.println("scan start");
  int n = WiFi.scanNetworks();
  Serial.println("scan done");
  if (n > 0) {
    for (int i = 0; i < n; i++) {
      server.sendContent(String() + "\r\n<tr><td>SSID " + WiFi.SSID(i) + String((WiFi.encryptionType(i) == ENC_TYPE_NONE)?" ":" *") + " (" + WiFi.RSSI(i) + ")</td></tr>");
    }
  } else {
    server.sendContent(String() + "<tr><td>No WLAN found</td></tr>");
  }
  server.sendContent(
    "</table>"
    "\r\n<br /><form method='POST' action='save'><h4>Connect to network:</h4>"
    "<input type='text' placeholder='network' name='n'/>"
    "<br /><input type='password' placeholder='password' name='p'/>"
    "<br /><h4>Login details</h4>"
    "<br /><input type='text' placeholder='username' name='u'/>"
    "<br /><input type='password' placeholder='password' name='w'/>"
    "<br /><input type='submit' value='Save'/></form>"
    "</body></html>"
  );
  
  server.client().stop(); // Stop is needed because we sent no content length
}

/**Handles save to eeprom when in AP mode**/
void handleAPSave() {
  Serial.println("wifi save");
  
  server.arg("n").toCharArray(ssid, sizeof(ssid) - 1);
  server.arg("p").toCharArray(password, sizeof(password) - 1);
  server.arg("u").toCharArray(www_username, sizeof(www_username) - 1);
  server.arg("w").toCharArray(www_password, sizeof(www_password) - 1);
  
  server.sendHeader("Location", "", true);
  server.sendHeader("Cache-Control", "no-cache, no-store, must-revalidate");
  server.sendHeader("Pragma", "no-cache");
  server.sendHeader("Expires", "-1");
  
  server.send ( 302, "text/plain", "");  // Empty content inhibits Content-length header so we have to close the socket ourselves.
  server.client().stop(); // Stop is needed because we sent no content length
  
  saveCreds();
 
}

/**Handles the config save**/
void handleConfigSave() {
  if(!authed){
    if(!server.authenticate(loaded.www_name, loaded.www_pwd))
        return server.requestAuthentication(DIGEST_AUTH, www_realm, authFailResponse);
    authed = 1;
    prevMillis = millis();
  }

  local_numSen = server.arg("setNum").toInt();
  local_updateRate = (server.arg("updateRate").toInt()) * 60000;
  
  for(unsigned int i = 0; i < 6; i++){
    char temp[12];
    snprintf (temp, 12, "set%dName", i+1);
    server.arg(temp).toCharArray(local_senSetName[i], sizeof(local_senSetName[i]) - 1);
    
    snprintf (temp, 12, "channelID%d", i+1);
    local_channelID[i] =  atol(server.arg(temp).c_str());

    snprintf (temp, 12, "writekey%d", i+1);
    server.arg(temp).toCharArray(local_writekey[i], sizeof(local_writekey[i]) - 1);
  
    snprintf (temp, 12, "readkey%d", i+1);
    server.arg(temp).toCharArray(local_readkey[i], sizeof(local_readkey[i]) - 1);

    snprintf (temp, 12, "widget%d", i+1);
    local_widgetID[i] =  server.arg(temp).toInt();
  }

  server.arg("resNum").toCharArray(resultNum, sizeof(resultNum) - 1);

  Serial.print(local_numSen);
  Serial.print(local_updateRate);
  for(unsigned int i = 0; i < 6; i++){

    Serial.print(local_senSetName[i]);
    Serial.print(local_channelID[i]);
    Serial.print(local_writekey[i]); 
    Serial.print(local_readkey[i]); 
    Serial.print(local_widgetID[i]);

    Serial.println();
  }
  
  char temp[400];

  snprintf ( temp, 400,

"<html>\
    <head>\
        <meta http-equiv='refresh' content=\"1;url=/\" />\
        <title>Config update</title>\
        <style>\
          body { background-color: #cccccc; font-family: Arial, Helvetica, Sans-Serif; Color: #000088; }\
        </style>\
    </head>\
    <body>\
        <h1>Updating Config</h1>\
        <p>Redirecting in 1 seconds...</p>\
    </body>\
</html>"\
  );
  server.send ( 200, "text/html", temp );

  saveSensorConfig();
  loadedConfig = loadSensorConfig();
}

void handleNotFound() {
  
  String message = "File Not Found\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += ( server.method() == HTTP_GET ) ? "GET" : "POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";

  for ( uint8_t i = 0; i < server.args(); i++ ) {
    message += " " + server.argName ( i ) + ": " + server.arg ( i ) + "\n";
  }

  server.send ( 404, "text/plain", message );
  
}

/**Handles the toggle of send email state**/
void handleTogEmail() {
  if(!authed){
    if(!server.authenticate(loaded.www_name, loaded.www_pwd))
        return server.requestAuthentication(DIGEST_AUTH, www_realm, authFailResponse);
    authed = 1;
    prevMillis = millis();
  }
  
  char temp[400];

  snprintf ( temp, 400,

"<html>\
    <head>\
        <meta http-equiv='refresh' content=\"3;url=/\" />\
        <title>Toggle Email</title>\
        <style>\
          body { background-color: #cccccc; font-family: Arial, Helvetica, Sans-Serif; Color: #000088; }\
        </style>\
    </head>\
    <body>\
        <h1>Toggling email</h1>\
        <p>Redirecting in 3 seconds...</p>\
    </body>\
</html>"\
  );
  server.send ( 200, "text/html", temp );
  
  sendEmail = (sendEmail?false:true);
  saveSendEmail(sendEmail);
}

/**Handles the toggle of send SMS state**/
void handleTogSMS() {
  if(!authed){
    if(!server.authenticate(loaded.www_name, loaded.www_pwd))
        return server.requestAuthentication(DIGEST_AUTH, www_realm, authFailResponse);
    authed = 1;
    prevMillis = millis();
  }
  
  char temp[400];

  snprintf ( temp, 400,

"<html>\
    <head>\
        <meta http-equiv='refresh' content=\"3;url=/\" />\
        <title>Toggle SMS</title>\
        <style>\
          body { background-color: #cccccc; font-family: Arial, Helvetica, Sans-Serif; Color: #000088; }\
        </style>\
    </head>\
    <body>\
        <h1>Toggling SMS</h1>\
        <p>Redirecting in 3 seconds...</p>\
    </body>\
</html>"\
  );
  server.send ( 200, "text/html", temp );
  
  sendSMS = (sendSMS?false:true);
  saveSendSMS(sendSMS);
}

/**Handles the result count**/
void handleResNum() {
  if(!authed){
    if(!server.authenticate(loaded.www_name, loaded.www_pwd))
        return server.requestAuthentication(DIGEST_AUTH, www_realm, authFailResponse);
    authed = 1;
    prevMillis = millis();
  }
  Serial.println();
  Serial.println(resultNum);
  server.arg("resNum").toCharArray(resultNum, sizeof(resultNum) - 1);
  Serial.println(resultNum);
  
  char temp[400];

  snprintf ( temp, 400,

"<html>\
    <head>\
        <meta http-equiv='refresh' content=\"1;url=/\" />\
        <title>Result update</title>\
        <style>\
          body { background-color: #cccccc; font-family: Arial, Helvetica, Sans-Serif; Color: #000088; }\
        </style>\
    </head>\
    <body>\
        <h1>Updating results</h1>\
        <p>Redirecting in 1 seconds...</p>\
    </body>\
</html>"\
  );
  server.send ( 200, "text/html", temp );
}

/**Handles config page**/
void handleConfig() {
  if(!authed){  //server authentication message
    if(!server.authenticate(loaded.www_name, loaded.www_pwd))
        return server.requestAuthentication(DIGEST_AUTH, www_realm, authFailResponse);

    authed = 1;
    prevMillis = millis();  
  }     
  
  char temp[1100];

    snprintf ( temp,1100,
    "<html>\
      <head>\
        <title>System Configuration</title>\
        <style>\
          body { background-color: #cccccc; font-family: Arial, Helvetica, Sans-Serif; Color: #000088; }\
        </style>\
      </head>\
      <body>\
        <h1>System Configuration</h1>\
        <p>Email send state: %s</p>\
     <a href=\"/togemail\"><button>Toggle Email</button></a>\
      <p>SMS send state: %s</p>\
      <a href=\"/togsms\"><button>Toggle SMS</button></a>\
      <h2>Units configuration</h2>\
      <form method='POST' action='configsave'>\
        Enter number of sensor sets used (max 6): <input id = \"setNum\" type=\"number\" min=\"0\" step=\"1\" max = \"6\" value = \"%d\" name=\"setNum\"><br><br>\
        Enter update rate in minutes (recommended 5min with 6 sets and 4 sensors per set): <input id = \"updateRate\" type=\"number\" min=\"0\" step=\"1\" value = \"%d\" name=\"updateRate\"> min<br><br>\
    ",
    (sendEmail?"True":"False"), (sendSMS?"True":"False"), loadedConfig.numSen, (loadedConfig.updateRate)/(60000)
    );

    server.sendContent(temp);

    snprintf ( temp,1100,
    " Enter sensor set nickname:<br>\
    1: <input id = \"set1Name\" type=\"text\" placeholder = \"%s\" name=\"set1Name\">\
    2: <input id = \"set2Name\" type=\"text\" placeholder = \"%s\" name=\"set2Name\">\
    3: <input id = \"set3Name\" type=\"text\" placeholder = \"%s\" name=\"set3Name\"><br>\
    4: <input id = \"set4Name\" type=\"text\" placeholder = \"%s\" name=\"set4Name\">\
    5: <input id = \"set5Name\" type=\"text\" placeholder = \"%s\" name=\"set5Name\">\
    6: <input id = \"set6Name\" type=\"text\" placeholder = \"%s\" name=\"set6Name\"><br><br>\
    ",
      loadedConfig.senSetName[0], loadedConfig.senSetName[1], loadedConfig.senSetName[2], loadedConfig.senSetName[3], loadedConfig.senSetName[4], loadedConfig.senSetName[5]
    );

    server.sendContent(temp);

    snprintf ( temp,1100,
    "Enter channel ID's:<br>\
    1: <input id = \"channelID1\" type=\"number\" min=\"0\" step=\"1\" placeholder = \"%d\" name=\"channelID1\">\
    2: <input id = \"channelID2\" type=\"number\" min=\"0\" step=\"1\" placeholder = \"%d\" name=\"channelID2\">\
    3: <input id = \"channelID3\" type=\"number\" min=\"0\" step=\"1\" placeholder = \"%d\" name=\"channelID3\"><br>\
    4: <input id = \"channelID4\" type=\"number\" min=\"0\" step=\"1\" placeholder = \"%d\" name=\"channelID4\">\
    5: <input id = \"channelID5\" type=\"number\" min=\"0\" step=\"1\" placeholder = \"%d\" name=\"channelID5\">\
    6: <input id = \"channelID6\" type=\"number\" min=\"0\" step=\"1\" placeholder = \"%d\" name=\"channelID6\"><br><br>\
    ",
    loadedConfig.channelID[0], loadedConfig.channelID[1], loadedConfig.channelID[2], loadedConfig.channelID[3], loadedConfig.channelID[4], loadedConfig.channelID[5]
    );

    server.sendContent(temp);

    snprintf ( temp,1100,
    "Enter channel write key's:<br>\
    1: <input id = \"writekey1\" type=\"text\" placeholder = \"%s\" name=\"writekey1\">\
    2: <input id = \"writekey2\" type=\"text\" placeholder = \"%s\" name=\"writekey2\">\
    3: <input id = \"writekey3\" type=\"text\" placeholder = \"%s\" name=\"writekey3\"><br>\
    4: <input id = \"writekey4\" type=\"text\" placeholder = \"%s\" name=\"writekey4\">\
    5: <input id = \"writekey5\" type=\"text\" placeholder = \"%s\" name=\"writekey5\">\
    6: <input id = \"writekey6\" type=\"text\" placeholder = \"%s\" name=\"writekey6\"><br><br>\
    ",
    loadedConfig.writekey[0], loadedConfig.writekey[1], loadedConfig.writekey[2], loadedConfig.writekey[3], loadedConfig.writekey[4], loadedConfig.writekey[5]
    );

    server.sendContent(temp);

    snprintf ( temp,1100,
    "Enter channel read key's:<br>\
    1: <input id = \"readkey1\" type=\"text\" placeholder = \"%s\" name=\"readkey1\">\
    2: <input id = \"readkey2\" type=\"text\" placeholder = \"%s\" name=\"readkey2\">\
    3: <input id = \"readkey3\" type=\"text\" placeholder = \"%s\" name=\"readkey3\"><br>\
    4: <input id = \"readkey4\" type=\"text\" placeholder = \"%s\" name=\"readkey4\">\
    5: <input id = \"readkey5\" type=\"text\" placeholder = \"%s\" name=\"readkey5\">\
    6: <input id = \"readkey6\" type=\"text\" placeholder = \"%s\" name=\"readkey6\"><br><br>\
    ",
    loadedConfig.readkey[0], loadedConfig.readkey[1], loadedConfig.readkey[2], loadedConfig.readkey[3], loadedConfig.readkey[4], loadedConfig.readkey[5]
    );

    server.sendContent(temp);

    snprintf ( temp,1100,
    " <h2>Sensor configuration</h2>\
      Enter number of results: <input id = \"resNum\" type=\"number\" min=\"0\" step=\"1\" value = \"%s\" name=\"resNum\"><br><br>\
      Enter widget start number:<br>\
      1: <input id = \"widget1\" type=\"number\" min=\"0\" step=\"1\" placeholder = \"%d\" name=\"widget1\">\
      2: <input id = \"widget2\" type=\"number\" min=\"0\" step=\"1\" placeholder = \"%d\" name=\"widget2\">\
      3: <input id = \"widget3\" type=\"number\" min=\"0\" step=\"1\" placeholder = \"%d\" name=\"widget3\"><br>\
      4: <input id = \"widget4\" type=\"number\" min=\"0\" step=\"1\" placeholder = \"%d\" name=\"widget4\">\
      5: <input id = \"widget5\" type=\"number\" min=\"0\" step=\"1\" placeholder = \"%d\" name=\"widget5\">\
      6: <input id = \"widget6\" type=\"number\" min=\"0\" step=\"1\" placeholder = \"%d\" name=\"widget6\"><br><br>\
      <input type=\"submit\" value=\"Submit\">\
      </form>\
      </body>\
    </html>",
    resultNum, loadedConfig.widgetID[0], loadedConfig.widgetID[1], loadedConfig.widgetID[2], loadedConfig.widgetID[3], loadedConfig.widgetID[4], loadedConfig.widgetID[5]
     
    );

    server.sendContent(temp);
}

 /**Handle each sets webpage**/
void handleSet1() {
  if(!authed){  //server authentication message
    if(!server.authenticate(loaded.www_name, loaded.www_pwd))
        return server.requestAuthentication(DIGEST_AUTH, www_realm, authFailResponse);

    authed = 1;
    prevMillis = millis();  
  }     

  numSensors = (int)sensors[0].getDS18Count();
  
  char temp[1100];

  snprintf ( temp,1100,
    "<html>\
        <head>\
          <title>%s Information:</title>\
        <style>\
          body { background-color: #cccccc; font-family: Arial, Helvetica, Sans-Serif; Color: #000088; }\
        </style>\
        </head>\
      <body>\
        <h1>%s Information View:</h1>\
        <h2>Temperatures: </h2>\
        <p>Average sensor temp is: %.2f&deg;C</p>\
        <p></p>",
    loadedConfig.senSetName[0], loadedConfig.senSetName[0], avgT[0]
    );

    server.sendContent(temp);

    for(unsigned int i = 0; i< numSensors; i++){
      snprintf ( temp,1100,
      "<iframe width=\"450\" height=\"260\" style=\"border: 1px solid #cccccc;\" src=\"https://api.thingspeak.com/channels/%d/widgets/%d\"></iframe>\
      ",
       loadedConfig.channelID[0], loadedConfig.widgetID[0] + i
      );
  
      server.sendContent(temp);    

      if(((i+1)%2)==0 && i!=0)  server.sendContent("<br>");    
    }
  
    snprintf ( temp,1100,
    "<h2>History graphs: </h2>\
      <form method='POST' action='res'>\
       Enter number of results: <input id = \"resNum\" type=\"number\" min=\"0\" step=\"1\" value = \"%s\" name=\"resNum\"><br>\
      <input type=\"submit\" value=\"Submit\">\
      </form>\
    <p></p>", resultNum     
    );

    server.sendContent(temp);

    for(unsigned int i = 1; i<= numSensors; i++){
      snprintf ( temp,1100,
      "<iframe width=\"450\" height=\"260\" style=\"border: 1px solid #cccccc;\" src=\"https://api.thingspeak.com/channels/%d/charts/%d?bgcolor=%%23ffffff&color=%%23d62020&dynamic=true&api_key=%s&results=%s&title=Sensor+%d+Temperature&type=line&yaxis=%%C2%%B0C\"></iframe>\
      ", 
        loadedConfig.channelID[0], i, loadedConfig.readkey[0], resultNum, i
      );
  
      server.sendContent(temp);

      if((i%2)==0 && i!=0)  server.sendContent("<br>");    
    }
    
    snprintf ( temp,1100,
    "<h2>Export</h2>\
      Download all of %s feeds in CSV format.\
      <br><br>\
        <form action=\"https://api.thingspeak.com/stream/channels/%d/feeds?api_key=%s&amp;timezone=Africa/Johannesburg\" class=\"button_to\" method=\"post\">\
          <div>\
            <input class=\"btn btn-primary\" id=\"download_channel_csv_btn\" type=\"submit\" value=\"Download\" />\
          </div>\
        </form>\
      </body>\
    </html>", 
    loadedConfig.senSetName[0], loadedConfig.channelID[0], loadedConfig.writekey[0]   
    );

    server.sendContent(temp);
     
}
