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
  

server->on("/target", HTTP_POST, [](AsyncWebServerRequest *request) {}, 
    NULL, 
    [](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total) {

        // Préparation de l'envoi des données vers Node-RED
        if (parametre.target_ip != nullptr && parametre.target_port > 0) {
            HTTPClient http;
            String targetUrl = "http://" + String(parametre.target_ip) + ":" + String(parametre.target_port) + "/target";

            http.begin(targetUrl);
            http.addHeader("Content-Type", "application/json");
            
            // Envoyer `latestJSON` à Node-RED
            int httpResponseCode = http.POST(latestJSON);

            if (httpResponseCode > 0) {
                Serial.println("HTTP Response code: " + String(httpResponseCode));

                // Construction de la réponse JSON à envoyer au client
                DynamicJsonDocument doc(1024);
                doc["success"] = true;
                doc["message"] = "JSON sent to Node-RED successfully";
                doc["httpResponseCode"] = httpResponseCode;
                doc["sentJson"] = latestJSON; // Inclure le JSON envoyé dans la réponse
                
                String response;
                serializeJson(doc, response);

                // Envoyer la réponse JSON au client
                request->send(200, "text/plain", "Essai réussi");
                //request->send(200, "application/json", response);
            } else {
                Serial.println("Error on sending POST: " + String(httpResponseCode));
                request->send(500, "text/plain", "Failed to send JSON to Node-RED");
            }

            http.end();
        } else {
            request->send(500, "text/plain", "Node-RED server IP or Port not set correctly.");
        }
    }
);



    // If request doesn't match any route, returns 404.
    server->onNotFound([](AsyncWebServerRequest *request){
        request->send(404);
      });

server->on("/target", HTTP_POST, [](AsyncWebServerRequest *request) {}, NULL, 
  [](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total) {
    // Cette fonction est appelée une fois que le corps de la requête POST est entièrement reçu
    String receivedData = String((char*)data);
    Serial.println("Données reçues via POST sur /target: " + receivedData);

    // Traitez ici les données reçues si nécessaire. Exemple :
    // updateConfiguration(receivedData); // Une fonction hypothétique pour mettre à jour la configuration
    
    // Envoie le dernier JSON à Node-RED
    String targetUrl = "http://" + String(parametre.target_ip) + ":" + String(parametre.target_port) + "/target";
    HTTPClient http;
    http.begin(targetUrl);
    http.addHeader("Content-Type", "application/json");
    int httpResponseCode = http.POST(tampon.payload); // tampon.payload contient votre dernier JSON

    if (httpResponseCode > 0) {
      Serial.print("Réponse HTTP : ");
      Serial.println(httpResponseCode);
      request->send(200, "text/plain", "JSON envoyé avec succès à Node-RED");
    } else {
      Serial.print("Erreur lors de l'envoi du POST : ");
      Serial.println(httpResponseCode);
      request->send(500, "text/plain", "Erreur lors de l'envoi du JSON à Node-RED");
    }
    http.end();
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

void sendJsonToNodeRed() {
  if (parametre.target_ip != nullptr && parametre.target_port > 0) {
    HTTPClient http;
    String targetUrl = String("http://") + String(parametre.target_ip) + ":" + String(parametre.target_port) + "/target";
    
    http.begin(targetUrl);
    http.addHeader("Content-Type", "application/json");
    
    String jsonPayload = makeJSON2(); // Assurez-vous que cette fonction génère votre JSON correctement.
    int httpResponseCode = http.POST(jsonPayload);
    
    if (httpResponseCode > 0) {
      Serial.println("HTTP Response code: " + String(httpResponseCode));
    } else {
      Serial.print("Error on sending POST: ");
      Serial.println(httpResponseCode);
    }
    http.end();
  } else {
    Serial.println("Target IP or Port not set correctly.");
  }
}
