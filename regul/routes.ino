/* 
 * Auteur : G.Menez
 * Fichier : http_as_serverasync/routes.ino 
 */

#include "ESPAsyncWebServer.h"
#include "routes.h"
#include "SPIFFS.h"
#include <ArduinoJson.h>
#include <HTTPClient.h>


/*===================================================*/


String processor(const String& var){
  /*char buffer[20];
  
  if (var.equals("UPTIME")) {

    char buffer[100];
    sprintf(buffer, "%f ms",  String(millis()));
    return buffer ;
  }*/
  char buffer[20];
  if (var.equals("UPTIME")) {
    // Get uptime in milliseconds
    unsigned long currentMillis = millis();
    // Convert milliseconds to seconds with two decimal places
    float uptime = currentMillis / 1000.0;
    sprintf(buffer, "%.2f s", uptime);
    return buffer;
  }
  else if (var.equals("WHERE")) {
    char buffer[100];
    sprintf(buffer, "Salle : %s, Latitude : %s, Longitude : %s, Adresse : %s.",  parametre.room, parametre.lat ,  parametre.lon , parametre.address);
    return buffer ;
  }
  else if (var.equals("SSID")) {  
    return WiFi.SSID();
  }
  else if (var.equals("MAC")) {
    return WiFi.macAddress().c_str();
  }
  else if (var.equals("IP")) {
    return  WiFi.localIP().toString().c_str();
  }
  else if (var.equals("TEMPERATURE")) {
    return dtostrf(info.temperature, 10, 2, buffer); // Convertir le float en chaîne avec 10 chiffres maximum et 2 chiffres après la virgule
  }
  else if (var.equals("LIGHT")) {
    return dtostrf(info.lumiere, 10, 2, buffer); 
  }
  else if (var.equals("LT")) {
    return dtostrf(parametre.temperatureSeuilHaut, 10, 2, buffer); 
  }
  else if (var.equals("HT")) {
    return  dtostrf(parametre.temperatureSeuilBas, 10, 2, buffer); 
  }
  else if (var.equals("COOLER")) {
    return info.etatRegulateurTemperature==0 ? "true" : "false"; 
  }
  else if (var.equals("HEATER")) {
    return info.etatRegulateurTemperature==2 ? "true" : "false";
  }
  else {
    return String();
  }
}


/*===================================================*/
void setup_http_routes(AsyncWebServer* server) {

  // Serveur de fichiers statiques. Notez qu'il utilise 'processor' pour traiter les fichiers
  server->serveStatic("/", SPIFFS, "/").setDefaultFile("index.html").setTemplateProcessor(processor);

  // Gestionnaire de racine simplifié
  server->on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(SPIFFS, "/index.html", String(), false, processor);
  });

  // Gestionnaire pour recevoir des données HTML et les enregistrer dans un fichier sur SPIFFS
  server->on("/writeHtml", HTTP_POST, [](AsyncWebServerRequest *request) {
    if (request->hasArg("html")) {
      const char* target_html = request->arg("html").c_str();
      // Assurez-vous d'implémenter la fonction 'writeFile' pour écrire dans SPIFFS
      File file = SPIFFS.open("/index.html", FILE_WRITE);
      if (file) {
        file.print(target_html);
        file.close();
      }
    }
    request->send(200, "text/plain", "OK");
  });

  // Gestionnaire pour envoyer des données HTML stockées sur SPIFFS
  server->on("/getHtml", HTTP_GET, [](AsyncWebServerRequest *request) {
    File file = SPIFFS.open("/index.html", FILE_READ);
    if (file) {
      String fileContent = file.readString();
      file.close();
      request->send(200, "text/html", fileContent);
    } else {
      request->send(404, "text/plain", "File Not Found");
    }
  });


  server->on("/getJson", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(200, "application/json", makeJSON());
  });

  server->on("/setData", HTTP_POST, [](AsyncWebServerRequest *request) {
    // Réponse initiale pour la requête HTTP POST.
    request->send(200, "text/plain", "OK");
  }, NULL, [](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total) {
    // Convertit les données reçues en chaîne JSON.
    String jsonData = String((char*)data);

    if (jsonData.length() > 0) {
        // Utilise la fonction `updateFromReceivedJson` pour traiter les données.
        updateFromReceivedJson(jsonData.c_str());
        Serial.println("Données JSON reçues et traitées via HTTP.");
        request->send(200, "text/plain", "Données reçues et traitées");
    } else {
        request->send(400, "text/plain", "Bad Request - No Data");
    }
  });

  server->on("/setNetwork", HTTP_POST, [](AsyncWebServerRequest *request) {
    if (request->hasArg("box")) {
      const char* content = request->arg("box").c_str();
      Serial.println(content);
      setNetworkInfos(content);
    }
 
    request->send(200, "text/plain", "Données reçues et traitées");
  });

  server->on("/target", HTTP_POST, [](AsyncWebServerRequest *request) {
    // Récupérer le corps JSON de la requête
    String targetIpValue = request->arg("ip");
    String targetPortValue = request->arg("port");
    String spValue = request->arg("sp");

    // Convertir les valeurs en entiers
    char* target_ip = strdup(targetIpValue.c_str());
    int target_port = targetPortValue.toInt();
    int sp = spValue.toInt();

    // Affecter les valeurs à vos variables
    parametre.target_ip = target_ip;
    parametre.target_port = target_port;
    parametre.sp = sp;
    
    // Envoyer une réponse OK à la requête HTTP
    request->send(200, "text/plain", "Donnees envoyees vers le Reporthost sous format JSON.");
  });

  server->on("/target2", HTTP_GET, [](AsyncWebServerRequest *request){
    USE_SERIAL.printf("GET /temperature request\n");
    // Envoyer la température en tant que réponse
    request->send(200, "text/plain", "Message reçu avec succès");
  });
  
  // If request doesn't match any route, returns 404.
  server->onNotFound([](AsyncWebServerRequest *request){
      request->send(404);
    });

  server->on("/temperature", HTTP_GET, [](AsyncWebServerRequest *request){

    USE_SERIAL.printf("GET /temperature request\n");
  
    // Convertir la valeur de température en une chaîne de caractères
    char temperatureString[10]; // Supposons que la température ne dépasse pas 10 caractères
    snprintf(temperatureString, sizeof(temperatureString), "%.2f", info.temperature); // "%.2f" pour afficher 2 décimales
    
    // Envoyer la température en tant que réponse
    request->send(200, "text/plain", temperatureString);
    });

  server->on("/light", HTTP_GET, [](AsyncWebServerRequest *request){
    char lumiereString[10]; // Supposons que la température ne dépasse pas 10 caractères
    snprintf(lumiereString, sizeof(lumiereString), "%.2f", info.lumiere); // "%.2f" pour afficher 2 décimales
    request->send_P(200, "text/plain", lumiereString);
    });

  server->on("/cooler", HTTP_GET, [](AsyncWebServerRequest *request){
    char tmp[10]; // Supposons que la température ne dépasse pas 10 caractères
    snprintf(tmp, 100, "%s",info.etatRegulateurTemperature==0 ? "true" : "false"); // "%.2f" pour afficher 2 décimales
    request->send_P(200, "text/plain", tmp);
    });

  server->on("/heater", HTTP_GET, [](AsyncWebServerRequest *request){
    char tmp[10]; // Supposons que la température ne dépasse pas 10 caractères
    snprintf(tmp, 100, "%s", info.etatRegulateurTemperature==2 ? "true" : "false"); // "%.2f" pour afficher 2 décimales
    request->send_P(200, "text/plain", tmp);
  });

  server->on("/uptime", HTTP_GET, [](AsyncWebServerRequest *request) {
    char uptimeString[20];
    unsigned long uptime = millis();
    snprintf(uptimeString, sizeof(uptimeString), "%lu", uptime / 1000);
    request->send(200, "text/plain", uptimeString);
  });

}
