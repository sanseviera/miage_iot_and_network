char* makeJSON(){

  /* 1) Build the JSON object ... easily with API !*/
  JsonDocument jdoc;
     JsonDocument status;
    JsonDocument location;
      JsonDocument gps;
    JsonDocument regul;
    JsonDocument information;
    JsonDocument net;
    JsonDocument reporthost;

  /* 1) Build the JSON object ... easily with API !*/
  
  /* 1.1) Etage 3 */
  gps["lat"] = 43.62453842;
  gps ["lon"] = 43.62453842;
  
  /* 1.2) Etage 2 */
  status["temperature"] = info.temperature;
  status["temperatureMax"] = info.maxEnregistre;
  status["temperatureMin"] = info.minEnregistre;
  status["light"] = info.lumiere;
  status["regul"] = makeText(info.regulation == 1);
  status["fire"] = info.feu == 1;
  status["heat"] = makeText2(info.temperature < parametre.temperatureSeuilBas && info.regulation);
  status["cold"] = makeText2(info.temperature > parametre.temperatureSeuilHaut && info.regulation);
  status["fanspeed"] = info.vitesseVentilateur;

  location["room"] = 312;
  location["gps"] = gps; 
  location["address"] = "Les lucioles";

  regul["lt"] = parametre.temperatureSeuilHaut ;
  regul["ht"] = parametre.temperatureSeuilBas;
  regul["lumiereAlerte"] = parametre.lumiereAlerte;
  regul["temperatureAlerte"] = parametre.temperatureAlerte;
  regul["pourcentageAvantAlerte"] = parametre.pourcentageAvantAlerte;

  information["ident"] = "ESP32 123";
  information["user"] = "GM";
  information["loc"] = "A biot";

  net["uptime"] = String(millis());
  net["ssid"] = "Livebox-B870";
  net["mac"] = "AC:67:B2:37:C9:48";
  net["ip"] = "192.168.1.45";

  reporthost["target_ip"] = "127.0.0.1" ;
  reporthost["target_port"] = 1880 ;
  reporthost["sp"] = 2 ;

  /* 1.3) Etage 1 */
  
  jdoc["status"] =  status;
  jdoc["location"] =  location;
  jdoc["regul"] =  regul;
  jdoc["information"] =  information;
  jdoc["net"] =  net;
  jdoc["reporthost"] =  reporthost;

  /* 2) SERIALIZATION => fill the payload string from jdoc object */
  serializeJson(jdoc, tampon.payload);

  /* 3) Send the request to the network and Receive the answer */
  Serial.println(tampon.payload);
  return tampon.payload;

}


void updateFromReceivedJson(const char* json) {
  // Augmentez la taille si votre JSON est plus grand
  StaticJsonDocument<2048> doc; 
  DeserializationError error = deserializeJson(doc, json);

  if (error) {
    Serial.print(F("deserializeJson() failed: "));
    Serial.println(error.c_str());
    return;
  }

  JsonObject regul = doc["regul"].as<JsonObject>();
  if (!regul.isNull()) {
    if (regul.containsKey("lt")) {
      parametre.temperatureSeuilHaut = regul["lt"];
      //Serial.println("Temperature seuil haut (lt) mise à jour : ");
    }
    if (regul.containsKey("ht")) {
      parametre.temperatureSeuilBas = regul["ht"];
      //Serial.println("Temperature seuil bas (ht) mise à jour : ");
    }
    if (regul.containsKey("temperatureAlerte")) {
      parametre.temperatureAlerte = regul["temperatureAlerte"];
      //Serial.println("Temperature d'alerte mise à jour : ");
    }
    if (regul.containsKey("lumiereAlerte")) {
      parametre.lumiereAlerte = regul["lumiereAlerte"];
      //Serial.println("LumiereAlerte mise à jour : ");
    }
  }
}
