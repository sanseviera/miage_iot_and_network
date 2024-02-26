#include "OneWire.h"
#include "DallasTemperature.h"
#include <Adafruit_NeoPixel.h>
#include <ArduinoJson.h>

//-------------Structures---------------------    
struct Information { 
  int chanceFeu; // une valeur en pourcentage
  float lumiere;
  float temperature;
  int etatRegulateurTemperature; //  0) refroidi , 1) est éteind , 2) chauffe
  bool alerteEtatLed; // pour faire clignoter la led
  // Variable pour le TIMER
  long timerGeneral;
  long timerBandeLed;
  long timerCommunication;
  
};
struct Information info = {0 , 0.0 , 0.0 , 0, 0, 0.0, 0.0, 0.0};

struct Parametre{
  const int temperatureSeuilHaut = 25;
  const int temperatureSeuilBas = 22;
  const int lumiereAlerte = 3000;
  const float temperatureAlerte = 20;
  const int pourcentageAvantAlerte = 80; // Pourcentage à atteindre pour déclencher l'alerte.
  //----------------------------
  const int ledPin = 19;
  const int ledPinVerte = 16;
  const int ledPinJaune = 2;
  const int bandeLedPin = 5;
  const int brocheVentilateur = 26;
  const int brocheBande = 13;
  //----------------------------
  const double periodeTimerGeneral = 1000;
  const double periodeTimerBandeLed = 300;
  const double periodeTimerCommunication = 5000;
};
struct Parametre parametre = {};

//----------------------------------


OneWire oneWire(23);
DallasTemperature tempSensor(&oneWire);
int numberKeyPresses = 0;
Adafruit_NeoPixel strip(parametre.bandeLedPin, parametre.brocheBande, NEO_GRB + NEO_KHZ800);

void IRAM_ATTR isr() { // Interrupt Handler
  numberKeyPresses++;
} 


//--------------Fonction de base INITIALISATION--------------------

void initLed(int ledPin) {
  pinMode(parametre.ledPin, OUTPUT);
  pinMode(parametre.ledPinVerte, OUTPUT);
  pinMode(parametre.ledPinJaune, OUTPUT);

}

void initCapteurChaleur(){
    tempSensor.begin();
}

void initVentilo(){
  
  pinMode(parametre.brocheVentilateur, OUTPUT); // broche 26
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
    digitalWrite(parametre.ledPinVerte, HIGH);
    analogWrite(parametre.brocheVentilateur, info.temperature * 4);
  } else{
    digitalWrite(parametre.ledPinVerte, LOW);
    analogWrite(parametre.brocheVentilateur, 0);
  }
}


void setAlerte(){
  if(info.chanceFeu >= parametre.pourcentageAvantAlerte ){
    digitalWrite(parametre.ledPinJaune, HIGH);
  } else{
    digitalWrite(parametre.ledPinJaune, LOW);
  }
}

void setEtatRegulateurTemperature(){
  if(info.chanceFeu  > 80) // Si le risque de feu est trop grand on arrête tout
  {
    info.etatRegulateurTemperature = 1;
  } 
  else if(info.temperature < parametre.temperatureSeuilBas){
    info.etatRegulateurTemperature = 2;
  }
  else if(info.temperature > parametre.temperatureSeuilHaut){
    info.etatRegulateurTemperature = 0;
  }
  else{
    info.etatRegulateurTemperature = 1;
  }
}


int lireCapteurLumiere(){
  int sensorValue;
  sensorValue = analogRead(A5); // Read analog input on ADC1_CHANNEL_5 (GPIO 33)
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
 strip.setPixelColor(i, strip.Color(255, 0, 0));
 }
 for(int i=1; i<4; i++) {
 //turn color to green
 strip.setPixelColor(i, strip.Color(0, 255, 0));
 }
 for(int i=4; i<parametre.bandeLedPin; i++) {
 //turn color to green
 strip.setPixelColor(i, strip.Color(0, 0, 255));
 }
 strip.show();
}

void setLed(){
  int colorA = 0; 
  int colorB = 0; 
  int colorC = 0; 

  if(info.chanceFeu > 80){
    if(info.alerteEtatLed==1){
      info.alerteEtatLed = 0;
      colorA = 0;
      colorB = 0;
      colorC = 255;
    } else {
      info.alerteEtatLed =1;
      colorA = 0;
      colorB = 0;
      colorC = 0;
    }
    
  }
  else if(info.etatRegulateurTemperature == 0){
    colorA = 255;
    colorB = 0;
    colorC = 0;
  }
  else if(info.etatRegulateurTemperature == 1){
    colorA = 255;
    colorB = 153;
    colorC = 51;
  }
  else if (info.etatRegulateurTemperature == 2){
    colorA = 0;
    colorB = 204;
    colorC = 0;
  }
  else{

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
  Serial.printf("Lumière : %f\n",info.lumiere);
  Serial.printf("Chance de feu : %i%%\n",info.chanceFeu);
  Serial.print("-----------------------\n");
}

void makeJSON(){
  char payload[256]; 

  /* 1) Build the JSON object ... easily with API !*/
  StaticJsonDocument<256> jdoc; 
  jdoc["chanceFeu"] = info.chanceFeu;
  jdoc["lumiere"] = info.lumiere;
  jdoc["temperature"] = info.temperature;
  jdoc["etatRegulateurTemperature"] = info.etatRegulateurTemperature; 
  jdoc["time"] =  millis();


  /* 2) SERIALIZATION => fill the payload string from jdoc object */
  serializeJson(jdoc, payload);

  /* 3) Send the request to the network and Receive the answer */
  Serial.println("Emission of the String/Payload ");
  Serial.println(payload);
}

//-------------Fonction native---------------------

void setup(){
  Serial.begin(9600);
  initLed(parametre.ledPin);
  initCapteurChaleur();
  initVentilo();
  setBandeLed();
  delay(1000);
}

void loop() {
  // Ce type de condition permet d'exécuter le contenu toutes les N millisecondes, ce qui évite d'avoir un programme trop linéaire.
  if(info.timerGeneral == 0 || millis() - info.timerGeneral > parametre.periodeTimerGeneral){

    info.timerGeneral=millis();
    info.lumiere = lireCapteurLumiere();
    info.temperature = lireCapteurChaleur();

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
    informationPrint();
    makeJSON();
  }
  

}
