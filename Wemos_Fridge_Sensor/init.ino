void STA(){
  WiFi.mode ( WIFI_STA );
  WiFi.begin ( loaded.ssid, loaded.ssid_pwd);
  Serial.println ( "" );
  
  // Wait for connection
  while ( WiFi.status() != WL_CONNECTED ) {
    delay ( 500 );
    Serial.print ( "." );
  }

  Serial.println ( "" );
  Serial.print ( "Connected to " );
  Serial.println ( loaded.ssid );
  Serial.print ( "IP address: " );
  Serial.println ( WiFi.localIP() );

  if ( MDNS.begin ( "esp8266" ) ) {
    Serial.println ( "MDNS responder started" );
  }

  server.on ( "/", handleRoot );
  server.on ( "/togemail", handleTogEmail );
  server.on ( "/res", handleResNum );
  server.onNotFound ( handleNotFound );
  server.begin();
  Serial.println ( "HTTP server started" );

  Serial.println();
  Serial.println(loaded.ssid);
  Serial.println(loaded.ssid_pwd);
  Serial.println(loaded.www_name);
  Serial.println(loaded.www_pwd);
  
}
void AP(){
  WiFi.mode ( WIFI_AP );
  WiFi.softAPConfig(apIP, apIP, netMsk);
  WiFi.softAP("FridgeInit", "SETUPPASSWORD");

  if ( MDNS.begin ( "esp8266" ) ) {
    Serial.println ( "MDNS responder started" );
  }
  Serial.println("Test init before server setup");
  server.on ( "/", handleRootAP );
  server.on ( "/save", handleAPSave );
  server.onNotFound ( handleNotFound );
  server.begin();
  Serial.println("Test init after server setup");
}

