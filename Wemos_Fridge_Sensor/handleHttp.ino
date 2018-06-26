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
    <p>Fridge temp is: %.2f &deg;C</p>\
    <p>Email send state: %s</p>\
    <a href=\"/togemail\"><button style=\"height: 100px; width: 300px; margin: 30px; auto 0\" >Toggle Email</button></a>\
    <p>Temperatures: </p>\
    <p></p>\
    <iframe width=\"450\" height=\"260\" style=\"border: 1px solid #cccccc;\" src=\"https://api.thingspeak.com/channels/521572/widgets/4796\"></iframe>\
    <iframe width=\"450\" height=\"260\" style=\"border: 1px solid #cccccc;\" src=\"https://api.thingspeak.com/channels/521572/widgets/4797\"></iframe>\
    <p>History graphs: </p>\
    <form method='POST' action='res'>\
      Enter number of results: <input id = \"resNum\" type=\"number\" min=\"0\" step=\"1\" value = \"%s\" name=\"resNum\"><br>\
      <input type=\"submit\" value=\"Submit\">\
    </form>\
    <p></p>\
    <iframe width=\"450\" height=\"260\" style=\"border: 1px solid #cccccc;\" src=\"https://api.thingspeak.com/channels/521572/charts/1?bgcolor=\%%23ffffff&color=%%23d62020&dynamic=true&api_key=CLEMYGM2K0KHL1MI&results=%s&title=Sensor+1+Temperature&type=line&yaxis=%%C2%%B0C\"></iframe>\
    <iframe width=\"450\" height=\"260\" style=\"border: 1px solid #cccccc;\" src=\"https://api.thingspeak.com/channels/521572/charts/2?bgcolor=\%%23ffffff&color=%%23d62020&dynamic=true&api_key=CLEMYGM2K0KHL1MI&results=%s&title=Sensor+2+Temperature&type=line&yaxis=%%C2%%B0C\"></iframe>\
    <h2>Export</h2>\
      Download all of this Channel feeds in CSV format.\
      <br><br>\
      <form action=\"https://api.thingspeak.com/stream/channels/521572/feeds?api_key=FVQ66PSAG1XF80RA&amp;timezone=Africa/Johannesburg\" class=\"button_to\" method=\"post\">\
    <div>\
    <input class=\"btn btn-primary\" id=\"download_channel_csv_btn\" type=\"submit\" value=\"Download\" />\
    <input name=\"authenticity_token\" type=\"hidden\" value=\"MBBZkRUNJXTxaelyayrhVx0AHeWUj9aBIzWUh73qGos=\" />\
    </div>\
    </form>\
  </body>\
</html>",

    hr, min % 60, sec % 60, avgT, (sendEmail?"True":"False"), resultNum, resultNum, resultNum
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
