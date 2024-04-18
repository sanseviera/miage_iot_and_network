#include <ArduinoJson.h>  // Assurez-vous d'inclure cette bibliothèque en haut de votre fichier

void mqtt_pubcallback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");

  char msg[length + 1];
  for (unsigned int i = 0; i < length; i++) {
    msg[i] = (char)payload[i];
  }
  msg[length] = '\0'; // Terminateur nul pour finir le tableau de char
  Serial.println(msg);

  // Désérialisation du JSON
  StaticJsonDocument<1024> doc; // Augmentez la taille si nécessaire
  DeserializationError error = deserializeJson(doc, msg);
    
  if (!error) {
    if (String(topic) == parametre.topic_etat) {
      int etatPiscine = doc["etatPiscine"];  // Make sure the key matches what's sent by your device
      //parametre.piscineEtat = etatPiscine;
      //setLedPiscine();  // Update the LED based on the pool status
      if (etatPiscine != parametre.piscineEtat) {
            parametre.piscineEtat = etatPiscine;
            setLedPiscine();
        }
    }
  } else {
    Serial.print("Erreur de désérialisation: ");
    Serial.println(error.c_str());
    return;
  }

  float temperature = doc["status"]["temperature"];

  // Extraction de la température depuis le JSON
  //float temperature2 = doc["status"]["temperature"];  // Accès au chemin complet dans le document JSON
  Serial.print("Temperature: ");
  Serial.println(temperature, 2);  // Affiche la température avec 2 décimales

  float latitude = doc["location"]["gps"]["lat"];  // Accès au chemin complet dans le document JSON
  Serial.print("Latitude: ");
  Serial.println(latitude, 6);  // Affiche la température avec 2 décimales

  float longitude = doc["location"]["gps"]["lon"];  // Accès au chemin complet dans le document JSON
  Serial.print("Longitude: ");
  Serial.println(longitude, 6);  // Affiche la température avec 2 décimales

  const char* ident = doc["info"]["ident"]; // Ajouter l'identifiant ESP si disponible

  if (ident != nullptr) {
    addReceivedTemperature(temperature, latitude, longitude, ident); // Passer l'identifiant ici
  }
  
}

/*============= SUBSCRIBE to TOPICS ===================*/
void mqtt_subscribe_mytopics() {
  /*
   * Subscribe to MQTT topics 
   * There is no way on checking the subscriptions from a client. 
   * But you can also subscribe WHENEVER you connect. 
   * Then it is guaranteed that all subscriptions are existing.
   * => If the client is already connected then we have already subscribe
   * since connection and subscriptions go together 
   */
  // Checks whether the client is connected to the MQTT server
  while (!mqttclient.connected()) { // Loop until we're reconnected
    USE_SERIAL.print("Attempting MQTT connection...");
    
    // Attempt to connect => https://pubsubclient.knolleary.net/api
    
    // Create a client ID from MAC address .. should be unique ascii string and different from all other devices using the broker !
    String mqttclientId = "ESP32-22016588";
    mqttclientId += WiFi.macAddress(); // if we need random : String(random(0xffff), HEX);
    if (mqttclient.connect(mqttclientId.c_str(), // Mqttclient Id when connecting to the server : 8-12 alphanumeric character ASCII
         NULL,   /* No credential */ 
         NULL))
      {
      USE_SERIAL.println("connected");
          
      // then Subscribe topics
      mqttclient.subscribe(parametre.topic_json, 1);
      mqttclient.subscribe(parametre.topic_etat);
    } 
    else { // Connection to broker failed : retry !
      USE_SERIAL.print("failed, rc=");
      USE_SERIAL.print(mqttclient.state());
      USE_SERIAL.println(" try again in 5 seconds");
      delay(5000); // Wait 5 seconds before retrying
    }
  } // end while
}
