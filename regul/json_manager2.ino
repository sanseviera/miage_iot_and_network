#include <ArduinoJson.h>

// Simulez des variables globales pour la compilation


// Exemple de fonction makeText() et makeText2() pour la compilation
const char* makeText(bool condition) { return condition ? "True" : "False"; }
const char* makeText2(bool condition) { return condition ? "Active" : "Inactive"; }

String latestJSON = "{}";

char* makeJSON2() {
  // Augmentez la taille si nécessaire
  DynamicJsonDocument doc(1024);

  // Construction de l'objet JSON
  JsonObject gps = doc.createNestedObject("gps");
  gps["lat"] = parametre.lat;
  gps["lon"] = parametre.lon;

  JsonObject status = doc.createNestedObject("status");
  status["temperature"] = info.temperature;
  // Ajoutez les autres champs de status ici
  status["temperatureMax"] = info.maxEnregistre;
  status["temperatureMin"] = info.minEnregistre;
  status["light"] = info.lumiere;
  status["regul"] = makeText(info.regulation == 1);
  status["fire"] = info.feu == 1;
  status["heat"] = makeText2(info.temperature < parametre.temperatureSeuilBas && info.regulation);
  status["cold"] = makeText2(info.temperature > parametre.temperatureSeuilHaut && info.regulation);
  status["fanspeed"] = info.vitesseVentilateur;

  JsonObject location = doc.createNestedObject("location");
  location["room"] = parametre.room;
  location["gps"] = gps; 
  location["address"] = parametre.address;

  JsonObject regul = doc.createNestedObject("regul");
  regul["lt"] = parametre.temperatureSeuilHaut ;
  regul["ht"] = parametre.temperatureSeuilBas;
  regul["lumiereAlerte"] = parametre.lumiereAlerte;
  regul["temperatureAlerte"] = parametre.temperatureAlerte;
  regul["pourcentageAvantAlerte"] = parametre.pourcentageAvantAlerte;

  JsonObject information = doc.createNestedObject("information");
  information["ident"] = "ESP32 123";
  information["user"] = "GM";
  information["loc"] = "A biot";

  JsonObject net = doc.createNestedObject("net");
  net["uptime"] = String(millis());
  net["ssid"] = strdup(WiFi.SSID().c_str());
  net["mac"] = strdup(WiFi.macAddress().c_str());
  net["ip"] = strdup(WiFi.localIP().toString().c_str());

  JsonObject reporthost = doc.createNestedObject("reporthost");
  reporthost["target_ip"] = parametre.target_ip ;
  reporthost["target_port"] = parametre.target_port ;
  reporthost["sp"] = parametre.sp ;

  /* 1.3) Etage 1 */

  JsonObject jdoc = doc.createNestedObject("jdoc");
  jdoc["status"] =  status;
  jdoc["location"] =  location;
  jdoc["regul"] =  regul;
  jdoc["information"] =  information;
  jdoc["net"] =  net;
  jdoc["reporthost"] =  reporthost;

  // Serialize le JsonDocument dans une chaîne
  String output;
  serializeJson(doc, latestJSON);
  Serial.println(output); // Pour débogage
    

  // Vous devez retourner un pointeur vers une chaîne qui reste valide après la sortie de cette fonction
  // Note: Ce n'est pas la bonne façon de gérer la mémoire dans de vrais projets, cela sert juste d'exemple
  static char jsonOutput[1024];
  strncpy(jsonOutput, output.c_str(), sizeof(jsonOutput));
  //return jsonOutput;
  // Puis, convertissez-le pour le retour (si nécessaire) ou envoyez-le directement
    return strdup(latestJSON.c_str());
}
