/* 
 * Auteur : G.Menez
 * Fichier : http_as_serverasync/routes.ino 
 */

#include "ESPAsyncWebServer.h"
#include "routes.h"
#include "SPIFFS.h"

extern String last_temp, last_light;

/*===================================================*/


String processor(const String& var){
  char buffer[20];
  if (var.equals("UPTIME")) {
    return "gg";
  }
  else if (var.equals("WHERE")) {
    return "uu";
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
    return "uu";
  }
  else if (var.equals("HEATER")) {
    return "uu";
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
    // Récupérer le corps JSON de la requête
    String ltValue = request->arg("lt");
    String htValue = request->arg("ht");
    String lumiereAlertValue = request->arg("lumiereAlert");
    String temperatureAlerteValue = request->arg("temperatureAlerte");

    // Convertir les valeurs en entiers
    int lt = ltValue.toInt();
    int ht = htValue.toInt();
    int lumiereAlert = lumiereAlertValue.toInt();
    float temperatureAlerte = temperatureAlerteValue.toFloat();

    // Affecter les valeurs à vos variables
    parametre.temperatureSeuilHaut = lt;
    parametre.temperatureSeuilBas = ht;
    parametre.lumiereAlerte = lumiereAlert;
    parametre.temperatureAlerte = temperatureAlerte;


    // Envoyer une réponse OK à la requête HTTP
    request->send(200, "text/plain", "OK");
  });

  server->on("/setNetwork", HTTP_POST, [](AsyncWebServerRequest *request) {
    // Récupérer le corps JSON de la requête
    String targetIpValue = request->arg("target_ip");
    String targetPortValue = request->arg("target_port");
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
    request->send(200, "text/plain", "OK");
  });

}
