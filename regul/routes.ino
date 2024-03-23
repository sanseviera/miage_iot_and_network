/* 
 * Auteur : G.Menez
 * Fichier : http_as_serverasync/routes.ino 
 */

#include "ESPAsyncWebServer.h"
#include "routes.h"
#include "SPIFFS.h"

#define USE_SERIAL Serial
extern String last_temp, last_light;

/*===================================================*/
String processor(const String& var){
  // Exemple de traitement de variable, retournez var si aucune substitution n'est nécessaire
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
    USE_SERIAL.println("Receive Request for a periodic report!");
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
  
   /*server->serveStatic("/", SPIFFS, "/").setTemplateProcessor(processor);  
   auto root_handler = server->on("/", HTTP_GET, [](AsyncWebServerRequest *request){
      request->send(SPIFFS, "/index.html", String(), false, processor); 
      request->send_P(200, "text/html", "try", processor); // if page_html was a string .   
   });
   
   server->on("/getHtml", HTTP_GET, [](AsyncWebServerRequest *request){
    //char* tmp = readFile(SPIFFS,"/index.html");
    request->send_P(200, "text/html", "oK" );
   });
   
   server->on("/writeHtml", HTTP_POST, [](AsyncWebServerRequest *request){
    Serial.println("Receive Request for a periodic report !"); 
    if (request->hasArg("html")) {
      const char* target_html = request->arg("html").c_str();
      writeFile(SPIFFS, "/index.html", target_html);
    }
    request->send_P(200,"text/plain", "ok" );
   });*/


}
/*===================================================*/
