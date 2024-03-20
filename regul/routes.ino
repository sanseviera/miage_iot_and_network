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
  /* 
   * Sets up AsyncWebServer and routes 
   */
  
  // doc => Serve files in directory "/" when request url starts with "/"
  // Request to the root or none existing files will try to server the default
  // file name "index.htm" if exists 
  // => premier param la route et second param le repertoire servi (dans le SPIFFS) 
  // Sert donc les fichiers css !  
  server->serveStatic("/", SPIFFS, "/").setTemplateProcessor(processor);  
  
  // Declaring root handler, and action to be taken when root is requested
  auto root_handler = server->on("/", HTTP_GET, [](AsyncWebServerRequest *request){
      /* This handler will download index.html (stored as SPIFFS file) and will send it back */
      request->send(SPIFFS, "/index.html", String(), false, processor); 
      // cf "Respond with content coming from a File containing templates" section in manual !
      // https://github.com/me-no-dev/ESPAsyncWebServer
      // request->send_P(200, "text/html", page_html, processor); // if page_html was a string .   
    });
  
  server->on("/temperature", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/plain", "eeee");
    });
}
/*===================================================*/
