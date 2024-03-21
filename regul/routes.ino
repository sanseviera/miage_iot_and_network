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

}

/*===================================================*/
void setup_http_routes(AsyncWebServer* server) {
  
   server->serveStatic("/", SPIFFS, "/").setTemplateProcessor(processor);  
   auto root_handler = server->on("/", HTTP_GET, [](AsyncWebServerRequest *request){
      request->send(SPIFFS, "/index.html", String(), false, processor); 
      request->send_P(200, "text/html", "try", processor); // if page_html was a string .   
   });
   
   server->on("/getHtml", HTTP_GET, [](AsyncWebServerRequest *request){
    char* tmp = readFile(SPIFFS,"/index.html");
    request->send_P(200, "text/html", tmp );
   });
   
   server->on("/writeHtml", HTTP_POST, [](AsyncWebServerRequest *request){
    Serial.println("Receive Request for a periodic report !"); 
    if (request->hasArg("html")) {
      const char* target_html = request->arg("html").c_str();
      writeFile(SPIFFS, "/index.html", target_html);
    }
    request->send_P(200,"text/plain", "ok" );
   });


}
/*===================================================*/
