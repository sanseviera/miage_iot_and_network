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
  if(var == "TEMPERATURE"){
    return "gg";
  }
  else if(var == "LIGHT"){
    return "uu";
  }
  return String();
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

}

/*
 *if (regul.containsKey("lt")) {
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





    parametre.temperatureSeuilHaut = request->arg("lt").c_str();
    parametre.temperatureSeuilBas = request->arg("ht").c_str();
    parametre.temperatureSeuilBas = request->arg("temperatureAlerte").c_str();
    parametre.temperatureSeuilBas = request->arg("temperatureAlerte").c_str();
 */
/*===================================================*/
