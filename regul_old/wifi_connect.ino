void functionWificonnect(){

  String hostname = "Mon petit objet ESP32";

  wifi_connect_multi(hostname);               
  
  if (WiFi.status() == WL_CONNECTED){
    Serial.print("\nWiFi connected : yes ! \n"); 
    wifi_printstatus(0);  
  } 
  else {
    Serial.print("\nWiFi connected : no ! \n"); 
  }
}