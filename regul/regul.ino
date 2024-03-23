
/* 
 * Auteur 1 : S.Rawa
 * Auteur 1 : T.Borréani
 * Many sources :
 => https://raw.githubusercontent.com/RuiSantosdotme/ESP32-Course/master/code/WiFi_Web_Server_DHT/WiFi_Web_Server_DHT.ino
 => https://randomnerdtutorials.com/esp32-dht11-dht22-temperature-humidity-web-server-arduino-ide/
 => Kevin Levy 
 => G.Menez
*/

// Bibliothéque externe
  #include "OneWire.h"
  #include "DallasTemperature.h"
  #include <Adafruit_NeoPixel.h>
  #include <ArduinoJson.h>
  #include <WiFi.h> // https://www.arduino.cc/en/Reference/WiFi
  #include <WiFiMulti.h>
  #include "SPIFFS.h"
  #include "FS.h"
  #include "ESPAsyncWebServer.h"
  #include <ArduinoOTA.h>
// Nos fichiers
  #include "colors.h"
  #include "wifi_utils.h"
  #include "wifi_connect.h"
  #include "file_manager.h"
  #include "routes.h"



//-------------Préprocesseur---------------------
#define FORMAT_SPIFFS_IF_FAILED true
#define USE_SERIAL Serial

// Pour régler les problèmes de compatibilités
#define Old 0



//-------------Structures---------------------    

/* La structure Information contient l'ensemble des informations des capteurs, des variables déduites des capteurs
 * et des variables spéciales permettant d'appeler les fonctions avec un timer différent entre elles
 */
struct Information { 
  //-----Variable d'informations------
  int chanceFeu; // une valeur en pourcentage
  float lumiere;
  float temperature;
  float maxEnregistre;
  float minEnregistre;
  int etatRegulateurTemperature; //  0) refroidi , 1) est éteind , 2) chauffe
  //-----Variables pour le TIMER-----
  float timerGeneral;
  float timerBandeLed;
  float timerCommunication;
  int vitesseVentilateur;
  int feu; // 1 ou 0, il y a un feu ou non
  int regulation; // 1 ou 0 on régul ou non
  
};
struct Information info = {0 , 0.0 , 0.0 , 0.0 , 100000.0 , 0, 0.0, 0.0, 0.0, 0 , 0};

/* La structure Parametre contient l'ensemble des paramètres par défaut réglables par l'utilisateur.
 * À noter qu'ils sont désormais modifiables depuis Node-RED.
 */
struct Parametre{

  //----------------------------
  int temperatureSeuilHaut = 25;
  int temperatureSeuilBas = 24;
  int lumiereAlerte = 3000;
  float temperatureAlerte = 20;
  const int pourcentageAvantAlerte = 80; // Pourcentage à atteindre pour déclencher l'alerte.
  //------------Gestion des broches----------------
  const int ledPin = 19;
  const int ledPinVerte = 16;
  const int ledPinBleue = 2;
  const int bandeLedPin = 5;
  const int brocheVentilateur = 27;
  const int brocheBande = 13;
  const int brocheLightIntensity = A5;
  const int brocheTermometre = 23;

  //--------Parametres des TIMER-----------
  const double periodeTimerGeneral = 1000;
  const double periodeTimerBandeLed = 300;
  const double periodeTimerCommunication = 5000;
};
struct Parametre parametre = {};

//-------------------------------------------------------


//-------------------------------------------------------



OneWire oneWire(parametre.brocheTermometre);
DallasTemperature tempSensor(&oneWire);
int numberKeyPresses = 0;
Adafruit_NeoPixel strip(parametre.bandeLedPin, parametre.brocheBande, NEO_GRB + NEO_KHZ800);

void setup_OTA(); // from ota.ino
// Create AsyncWebServer object on port 80
AsyncWebServer server(80);

//--------------Fonction de base INITIALISATION--------------------

/*
 * Configuration du mode de fonctionnement des broches utilisé pour les LED sur OUTPUT
 */
void initLed(int ledPin) {
  pinMode(parametre.ledPin, OUTPUT);
  pinMode(parametre.ledPinVerte, OUTPUT);
  pinMode(parametre.ledPinBleue, OUTPUT);
}

void initCapteurChaleur(){
    tempSensor.begin();
}


void initVentilo() {
  #if Old
    ledcAttach(27, 25000, 8); // Associe le canal PWM 0 à la broche GPIO 27
    ledcWrite(27, 255); 
    // pinMode(parametre.brocheVentilateur, OUTPUT); // Décommentez et utilisez si nécessaire
  #else
    ledcAttachPin(27, 0); 
    ledcSetup(0, 25000, 8);  
  #endif
}

//--------------Fonction de base--------------------

void setChauffage(){
  if(info.etatRegulateurTemperature == 2){
    digitalWrite(parametre.ledPin, HIGH);
  } else{
     digitalWrite(parametre.ledPin, LOW);
  }
}

void setVentilo(){
  if(info.etatRegulateurTemperature == 0){
    int tmp = 0;
    if(info.temperature >= parametre.temperatureSeuilHaut && info.temperature < parametre.temperatureSeuilHaut+1){
      info.vitesseVentilateur = 64;
      tmp = 64;
    } else if(info.temperature+1 >= parametre.temperatureSeuilHaut && info.temperature < parametre.temperatureSeuilHaut+2){
      info.vitesseVentilateur = 127;
      tmp = 127;
    } else if(info.temperature+2 >= parametre.temperatureSeuilHaut && info.temperature < parametre.temperatureSeuilHaut+3){
      info.vitesseVentilateur = 191;
      tmp = 191;
    } else {
      info.vitesseVentilateur = 255;
      tmp = 255;
    }
    digitalWrite(parametre.ledPinVerte, HIGH);
    ledcWrite(0, tmp);
  } else{
    digitalWrite(parametre.ledPinVerte, LOW);
    ledcWrite(0, 0);  }
}


void setAlerte(){
  if(info.chanceFeu >= parametre.pourcentageAvantAlerte ){
    info.feu = 1;
    digitalWrite(parametre.ledPinBleue, HIGH);
  } else{
    info.feu = 0;
    digitalWrite(parametre.ledPinBleue, LOW);
  }
}

void setEtatRegulateurTemperature(){
  if(info.chanceFeu  > parametre.pourcentageAvantAlerte) // Si le risque de feu est trop grand on arrête tout
  {
    info.regulation = 0;
    info.etatRegulateurTemperature = 1;
  } 
  else if(info.temperature < parametre.temperatureSeuilBas){
    info.regulation = 1;
    info.etatRegulateurTemperature = 2;
  }
  else if(info.temperature > parametre.temperatureSeuilHaut){
     info.regulation = 1;
    info.etatRegulateurTemperature = 0;
  }
  else{
    info.regulation = 0;
    info.etatRegulateurTemperature = 1;
  }
}

int lireCapteurLumiere(){
  int sensorValue;
  sensorValue = analogRead(parametre.brocheLightIntensity); // Read analog input on ADC1_CHANNEL_5 (GPIO 33)
  return sensorValue;
}

float lireCapteurChaleur(){
  float t;
  tempSensor.requestTemperaturesByIndex(0); 
  t = tempSensor.getTempCByIndex(0); 
  return t;
}

void setBandeLed(){
  strip.begin();
  delay(1);
 for(int i=0; i<1; i++) {
 //turn color to red
 strip.setPixelColor(i, strip.Color(rgball[3][0] , rgball[3][1], rgball[3][2]));
 }

 strip.show();
}

void setLed(){
  int colorA = 0; 
  int colorB = 0; 
  int colorC = 0; 
  
  if(info.temperature > parametre.temperatureSeuilBas && info.temperature < parametre.temperatureSeuilHaut){
    colorA = rgball[0][0] ;
    colorB = rgball[0][1] ;
    colorC = rgball[0][2] ;
  }
  else if(info.temperature < parametre.temperatureSeuilBas ){
    colorA = rgball[1][0] ;
    colorB = rgball[1][1] ;
    colorC = rgball[1][2] ;
  }
  else if (info.temperature > parametre.temperatureSeuilHaut ){
    colorA = rgball[2][0] ;
    colorB = rgball[2][1] ;
    colorC = rgball[2][2] ;
  }
  for(int i=0; i<5; i++) {
    strip.setPixelColor(i, strip.Color(colorA, colorB, colorC));
  }

    strip.show();
}

/* 
* Fonction qui permet d'augmenter ou de réduire la variable représentant
* le pourcentage de chances d'incendies.
*/
void setDetectorFire(){
  if(info.lumiere > parametre.lumiereAlerte && info.temperature > parametre.temperatureAlerte){
    info.chanceFeu = info.chanceFeu+10;
  }
  else{
    info.chanceFeu = info.chanceFeu-10;
  }

  if(info.chanceFeu<0){
    info.chanceFeu=0;
  }
  else if (info.chanceFeu>100){
    info.chanceFeu=100;
  }
}

void informationPrint(){
  Serial.printf("Température : %f°\n",info.temperature);
  Serial.printf("Lumière : %i\n",info.lumiere);
  Serial.printf("Chance de feu : %i%%\n",info.chanceFeu);
  Serial.print("-----------------------\n");
}

const char* makeText(int i){
  if(i){
    return "WALK";}
  else{return "HALT";}
}

const char* makeText2(int i){
  if(i){
    return "ON";}
  else{return "OFF";}
}


void readData() {
  if (Serial.available()) {
    String data_received = Serial.readStringUntil('\n');
    String tempSeuilHaut = data_received;
    int delimiterPos = data_received.indexOf(':');
    if (delimiterPos != -1) {
      String sbValueString = data_received.substring(delimiterPos + 1);
      sbValueString.trim();  // Supprimer les espaces inutiles
      if (!sbValueString.isEmpty()) {
        int tempSeuilBas = sbValueString.toInt();
        parametre.temperatureSeuilBas = tempSeuilBas;
      }
    }
  }
}
  
char* makeJSON(){
  char payload[2048]; 

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
  serializeJson(jdoc, payload);

  /* 3) Send the request to the network and Receive the answer */
  Serial.println(payload);
  return payload;

}

void setMaxTemperature(){
  if (info.maxEnregistre <= info.temperature){
    info.maxEnregistre =  info.temperature;
  }
}


void setMinTemperature(){
  if (info.minEnregistre >= info.temperature){
    info.minEnregistre =  info.temperature;
  }
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





//-------------Fonction native---------------------

void setup(){
  Serial.begin(9600);
  while(!Serial); //wait for a serial connection
  functionWificonnect();
  initLed(parametre.ledPin);
  initCapteurChaleur();
  initVentilo();
  setBandeLed();
  delay(2000);

   verifFile();
   delay(2000);
   //writeFile(SPIFFS, "/index.html", readFile(SPIFFS,"/test.html"));

  // Setup routes of the ESP Web server
  setup_http_routes(&server);

  //setup_http_routes(&server);
  // Start ESP Web server
  server.begin();
   

}


void loop() {
  // Ce type de condition permet d'exécuter le contenu toutes les N millisecondes, ce qui évite d'avoir un programme trop linéaire.
  if(info.timerGeneral == 0 || millis() - info.timerGeneral > parametre.periodeTimerGeneral){

    info.timerGeneral=millis();
    info.lumiere = lireCapteurLumiere();
    info.temperature = lireCapteurChaleur();
    
    setMaxTemperature();
    setMinTemperature();
    setDetectorFire();
    setEtatRegulateurTemperature();
    setChauffage();
    setVentilo();
    setAlerte();
    
  }
  if(info.timerBandeLed == 0 || millis() - info.timerBandeLed > parametre.periodeTimerBandeLed){
    info.timerBandeLed=millis();
    setLed();
  }
  if(info.timerCommunication == 0 || millis() - info.timerCommunication > parametre.periodeTimerCommunication){
    info.timerCommunication=millis();
    //readData();
    makeJSON();
  }

 // Lecture des données JSON reçues sur la connexion série
  if (Serial.available() > 0) {
    String incomingJson = Serial.readStringUntil('\n'); // Lit la chaîne JSON terminée par un retour à la ligne
    if (incomingJson.length() > 0) {
      updateFromReceivedJson(incomingJson.c_str()); // Traite le JSON reçu
    }
  }
  
}
