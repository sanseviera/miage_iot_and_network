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
  
   server->serveStatic("/", SPIFFS, "/").setTemplateProcessor(processor);  
   auto root_handler = server->on("/", HTTP_GET, [](AsyncWebServerRequest *request){
      int UPTIME = 1111;
      request->send(SPIFFS, "/index.html", String(), false, processor); 
      request->send_P(200, "text/html", "try", processor); // if page_html was a string .   
   });
   
   
   server->on("/writeHtml", HTTP_POST, [](AsyncWebServerRequest *request){
    Serial.println("Receive Request for a periodic report !"); 
    if (request->hasArg("html")) {
      const char* target_html = request->arg("html").c_str();
      writeFile(SPIFFS, "/index.html", target_html);
    }
    request->send_P(200,"text/plain", "ok" );
   });

   server->on("/light", HTTP_GET, [](AsyncWebServerRequest *request){
      /* The most simple route => hope a response with light value */ 
      request->send_P(200, "text/plain", "ee");
   });
   
   server->on("/temperature", HTTP_GET, [](AsyncWebServerRequest *request){
      /* The most simple route => hope a response with temperature value */ 
      USE_SERIAL.printf("GET /temperature request \n"); 
      /* Exemple de ce qu'il ne faut surtout pas écrire car yield + async => core dump !*/
      request->send_P(200, "text/plain", "fff");
    });

    server->on("/getJson", HTTP_GET, [](AsyncWebServerRequest *request){
      static char jsonBuffer[2048];
          char *json = makeJSON();
      strncpy(jsonBuffer, json, sizeof(jsonBuffer));
        free(json);

        // Envoyer le JSON depuis le tableau de caractères statique
        request->send(200, "application/json", jsonBuffer);

    });
    



}
/*===================================================*/
