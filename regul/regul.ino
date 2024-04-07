
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
  #include <HTTPClient.h>
  #include <PubSubClient.h>
  #include "wifi_utils.h"
  #include <WiFi.h>
  #include <PubSubClient.h>
// Nos fichiers
  #include "json_manager.h"
  #include "colors.h"
  #include "wifi_utils.h"
  #include "wifi_connect.h"
  #include "file_manager.h"
  #include "routes.h"
  #include <math.h>

//-------------Préprocesseur---------------------
#define FORMAT_SPIFFS_IF_FAILED true
#define USE_SERIAL Serial

// Pour régler les problèmes de compatibilités la passer si besoin de 1 à 0 et inverse
#define Old 1


//-------------Structures---------------------    

/* La structure Information contient l'ensemble des informations des capteurs, des variables déduites des capteurs
 * et des variables spéciales permettant d'appeler les fonctions avec un timer différent entre elles
 */
struct Information { 
  char hostname[30]  ; 
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
  //-----
};
struct Information info = {"diop_borreani", 0 , 0.0 , 0.0 , 0.0 , 100000.0 , 0, 0.0, 0.0, 0.0, 0 , 0};

struct Tampon{
  char payload[2048]; 
};
struct Tampon tampon = {""};

/* La structure Parametre contient l'ensemble des paramètres par défaut réglables par l'utilisateur.
 * À noter qu'ils sont désormais modifiables depuis Node-RED.
 */
struct Parametre{
  

  //-----------Parametres du régulateur--------------
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
  //--------------Cible-------------------
  char* target_ip = "172.20.10.2";
  int target_port = 1883;
  int sp = 10;
  //--------------Lieu------------------
  char* room = "312";
  char* lat = "43.7102271";
  char* lon = "7.2599507";
  char* address = "12 Boulevard Joseph Garnier";
  //--------------Mqtt------------------
  char* mqtt_server = "test.mosquitto.org"; 
  // topic
  char* topic_temp = "uca/M1/iot/temp";
  char* topic_led = "uca/M1/iot/led";
  char* topic_json = "uca/iot/piscine";

};
struct Parametre parametre = {};
bool hspot = false;
bool occupe = false;

//-------------------------------------------------------


//-------------------------------------------------------
WiFiClient espClient; // Wifi
PubSubClient mqttclient(espClient); // MQTT client
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
/*
 * Initialisation du capteur de chaleur
 */
void initCapteurChaleur(){
    tempSensor.begin();
}
/*
 * Initialisation du ventilateur
 */
void initVentilo() {
  // Si ancienne version
  #if Old
    ledcAttach(27, 25000, 8); // Associe le canal PWM 0 à la broche GPIO 27
    ledcWrite(27, 255); 
    // pinMode(parametre.brocheVentilateur, OUTPUT); // Décommentez et utilisez si nécessaire
  // Si nouvelle version
  #else
    ledcAttachPin(27, 0); 
    ledcSetup(0, 25000, 8);  
  #endif
}

//--------------Fonction de base--------------------

/*
 * Pemet de gérer l'état de la led rouge représentant le chauffage
 */
void setChauffage(){
  if(info.etatRegulateurTemperature == 2){
    digitalWrite(parametre.ledPin, HIGH);
  } else{
     digitalWrite(parametre.ledPin, LOW);
  }
}

/*
 * Pemet de gérer l'état de la led verte représentant le ventillo ainsi que le réel ventilateur (ventilateur à plusieurs vitesses)
 */
void setVentilo(){
  // Si on est dans l'état correspondant au refroidissement 
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
  } 
  // Si on est dans un autre état
  else{
    digitalWrite(parametre.ledPinVerte, LOW);
    ledcWrite(0, 0);  }
}

/*
 * Si le pourcentage de chance de feu a passé un certain  seuil, on active l'alerte (l'alerte est la led bleu) sinon on la désactive
 */
void setAlerte(){
  if(info.chanceFeu >= parametre.pourcentageAvantAlerte ){
    info.feu = 1;
    digitalWrite(parametre.ledPinBleue, HIGH);
  } else{
    info.feu = 0;
    digitalWrite(parametre.ledPinBleue, LOW);
  }
}

/*
 * Cette fonction permet de gérer l'état du régulateur de température et nottament de couper le système en cas de risque incendie
 * 3 valeurs possible :
 *    - 0 si l'ont chauffe
 *    - 1 si rien n'est activé
 *    - 2 si l'ont refroidi
 *    
 */
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

/*
 * renvois la valeur que retourne le capteur de lumière
 */
int lireCapteurLumiere(){
  int sensorValue;
  sensorValue = analogRead(parametre.brocheLightIntensity); // Read analog input on ADC1_CHANNEL_5 (GPIO 33)
  return sensorValue;
}

/*
 * renvois la valeur que retourne le capteur de chaleur
 */
float lireCapteurChaleur(){
  float t;
  tempSensor.requestTemperaturesByIndex(0); 
  t = tempSensor.getTempCByIndex(0); 
  return t;
}

/*
 * Permet de gérer la bande led en switchant entre trois couleur différente
 */
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

  // si on est a une température entre les deux seuils
  if(info.temperature > parametre.temperatureSeuilBas && info.temperature < parametre.temperatureSeuilHaut){
    colorA = rgball[0][0] ;
    colorB = rgball[0][1] ;
    colorC = rgball[0][2] ;
  }
  // si on est a une température en dessous du seuil bas
  else if(info.temperature < parametre.temperatureSeuilBas ){
    colorA = rgball[1][0] ;
    colorB = rgball[1][1] ;
    colorC = rgball[1][2] ;
  }
   // si on est a une température au dessus du seuil haut
  else if (info.temperature > parametre.temperatureSeuilHaut ){
    colorA = rgball[2][0] ;
    colorB = rgball[2][1] ;
    colorC = rgball[2][2] ;
  }
  // parcours les 5 leds pour les régler
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

/*
 * Permet d'afficher sur le port serie des informations pour le debugage
*/
void informationPrint(){
  Serial.printf("Température : %f°\n",info.temperature);
  Serial.printf("Lumière : %i\n",info.lumiere);
  Serial.printf("Chance de feu : %i%%\n",info.chanceFeu);
  Serial.print("-----------------------\n");
}

// permet de transformer un boolean en chaîne
const char* makeText(int i){
  if(i){
    return "WALK";
  }
  else{
    return "HALT";
  }
}

// permet de transformer un boolean en chaîne
const char* makeText2(int i){
  if(i){
    return "ON";
    }
  else{
    return "OFF";
    }
}

/*
 * permet de lire de la donnée
 */
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

/*
 * permet de mettre a jours la temperature maximal enregistré
 */
void setMaxTemperature(){
  if (info.maxEnregistre <= info.temperature){
    info.maxEnregistre =  info.temperature;
  }
}

/*
 * permet de mettre a jours la temperature minimal enregistré
 */
void setMinTemperature(){
  if (info.minEnregistre >= info.temperature){
    info.minEnregistre =  info.temperature;
  }
}

struct TemperatureData {
    float receivedTemperatures[15]; // Stocke jusqu'à 10 températures reçues pour simplifier
    int count; // Nombre de températures reçues
    float latitudes[15]; // Pour stocker la latitude de chaque température reçue
    float longitudes[15]; // Pour stocker la longitude de chaque température reçue
} tempData;

void resetTemperatureData() {
    tempData.count = 0;
    hspot = false;
    for(int i = 0; i < 15; i++) {
        tempData.receivedTemperatures[i] = -999.0; // Initialise avec une valeur non valide
        tempData.latitudes[i] = -999.0; // Initialise avec une valeur non valide
        tempData.longitudes[i] = -999.0; // Initialise avec une valeur non valide
    }
}

double calculateDistance(double lat1, double lon1, double lat2, double lon2) {
    double R = 6371; // Rayon de la Terre en km
    double dLat = (lat2 - lat1) * (M_PI / 180);
    double dLon = (lon2 - lon1) * (M_PI / 180);
    double a = sin(dLat/2) * sin(dLat/2) +
               cos(lat1 * (M_PI / 180)) * cos(lat2 * (M_PI / 180)) * 
               sin(dLon/2) * sin(dLon/2);
    double c = 2 * atan2(sqrt(a), sqrt(1-a));
    double distance = R * c; // Distance en km
    return distance;
}

void addReceivedTemperature(float temperature, float latitude, float longitude) {
    if (tempData.count < 15) {
        tempData.receivedTemperatures[tempData.count] = temperature;
        tempData.latitudes[tempData.count] = latitude;
        tempData.longitudes[tempData.count] = longitude;
        tempData.count++;
    }
}

double myLat = atof(parametre.lat);
double myLon = atof(parametre.lon);

/*void checkAndSetHotspot() {
    //tempData.isHotspot = true; // Supposons que c'est un hotspot jusqu'à preuve du contraire
    
    for (int i = 0; i < tempData.count; i++) {
      double distance = calculateDistance(myLat, myLon, tempData.latitudes[i], tempData.longitudes[i]);
        if (distance <= 10.0 && tempData.receivedTemperatures[i] > info.temperature) {
            digitalWrite(2, LOW);  // Éteint la LED sur la broche 2
            hspot = false; // Si une température reçue est plus élevée, ce n'est pas un hotspot
            Serial.println("Nous ne sommes pas un hotspot.");
            //break;
        } else if (distance <= 10.0 && tempData.receivedTemperatures[i] < info.temperature) {
            digitalWrite(2, HIGH); // Allume la LED sur la broche 2
            hspot = true; // Si une température reçue est plus élevée, ce n'est pas un hotspot
            Serial.println("Nous sommes un hotspot !");
            //break;
        }
    }
}*/

void checkAndSetHotspot() {
    float maxTemperature = -999; // Initialise avec une valeur basse pour être sûr de trouver une température plus élevée

    // Trouvez la température maximale parmi les températures reçues dans un rayon de 10 km
    for (int i = 0; i < tempData.count; i++) {
        double distance = calculateDistance(myLat, myLon, tempData.latitudes[i], tempData.longitudes[i]);
        if (distance <= 10.0) {
            if (tempData.receivedTemperatures[i] > maxTemperature) {
                maxTemperature = tempData.receivedTemperatures[i];
            }
        }
    }

    // Comparez cette température maximale avec la température de l'ESP
    if (info.temperature > maxTemperature) {
        digitalWrite(2, HIGH); // Allume la LED sur la broche 2 pour indiquer que c'est un hotspot
        hspot = true;
        Serial.println("Nous sommes un hotspot !");
    } else {
        digitalWrite(2, LOW);  // Éteint la LED sur la broche 2, ce n'est pas un hotspot
        hspot = false;
        Serial.println("Nous ne sommes pas un hotspot.");
    }
}


void checkAndSetOccuped() {
        // Détection de présence basée sur la luminosité
        if (info.lumiere < 3000) {
          occupe = true;
          Serial.println("Nous sommes occupés.");
        } else {
          occupe = false;
          Serial.println("Nous ne sommes pas occupés.");
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
  
   // Setup routes of the ESP Web server
   setup_http_routes(&server);
  
   //setup_http_routes(&server);
   //Start ESP Web server
   server.begin();
  
   mqttclient.setBufferSize(1024);
  
   
   mqttclient.setServer(parametre.mqtt_server, 1883);
   mqttclient.setCallback(mqtt_pubcallback); 
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

    //int32_t period = 5000; // 5 sec
  
    /*--- subscribe to TOPIC_LED if not yet ! */
    /*mqtt_subscribe_mytopics();
    char payload[100];

    USE_SERIAL.print("Publish payload : "); USE_SERIAL.print(payload); 
    USE_SERIAL.print(" on topic : "); USE_SERIAL.println(parametre.topic_json);
    /*strncpy(payload, makeJSON(), sizeof(payload));
    Serial.print("-----------zzz---------");
    Serial.print(payload);
    Serial.print("--------------------");*/

    /*mqttclient.publish(parametre.topic_json, payload);

    /* Process MQTT ... une fois par loop() ! */
    //mqttclient.loop(); // Process MQTT event/action

    mqtt_subscribe_mytopics(); // Assurez-vous que vous êtes abonné à vos topics si nécessaire
  
    char* payload = makeJSON(); // Génère le JSON
  
    USE_SERIAL.print("Publish payload : "); USE_SERIAL.println(payload); 
    USE_SERIAL.print(" on topic : "); USE_SERIAL.println(parametre.topic_json);
  
    mqttclient.publish(parametre.topic_json, payload);

    checkAndSetOccuped();
    checkAndSetHotspot();
    
    mqttclient.loop(); // Gère les callbacks et maintient la connexion active
 
  

  }

  // Lecture des données JSON reçues sur la connexion série
  if (Serial.available() > 0) {
    String incomingJson = Serial.readStringUntil('\n'); // Lit la chaîne JSON terminée par un retour à la ligne
    if (incomingJson.length() > 0) {
      updateFromReceivedJson(incomingJson.c_str()); // Traite le JSON reçu
    }
  }

  unsigned long previousMillis = 0; // Dernier moment où la requête a été envoyée
  unsigned long currentMillis = millis(); // Intervalle entre les requêtes (en millisecondes, 10000ms = 10s)

  /*if (currentMillis - previousMillis >= parametre.sp*10000) {
    // Sauvegardez le moment de l'envoi
    previousMillis = currentMillis;
    
    HTTPClient http;
    char* url = (char*)malloc(10000 * sizeof(char)); // Allouer de la mémoire pour l'URL
    snprintf(url, 5000, "http://%s:%d/target", parametre.target_ip, parametre.target_port);
    http.begin(url);
    String requestBody = makeJSON();
    int httpCode = http.POST(requestBody);
  
    Serial.printf("[HTTP] POST... code: %d\n", httpCode);
    // N'oubliez pas de libérer la mémoire allouée pour l'URL
    free(url);
    // Fermez la connexion après l'envoi
    http.end();
  }*/
  
}

#if 0
/*
 *  Old fashion payload and publishing
 *
 */
  char payload[100];
  char topic[150];
  //
  payload = makeJSON();
  
  client.publish(topic, payload);
#endif
