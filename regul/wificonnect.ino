/*** Basic/Static Wifi connection
     Fichier wificonnect/wificonnect.ino ***/

#include <WiFi.h> // https://www.arduino.cc/en/Reference/WiFi

#include "wifi_utils.h"

#define USE_SERIAL Serial

/*--------------------------------------------------------------------------* 
  Arduino IDE paradigm : setup+loop 
  *--------------------------------------------------------------------------*/ 
void functionWificonnect(){
  /* Serial connection -----------------------*/
  Serial.begin(9600);
  while(!Serial); //wait for a serial connection  

  /* WiFi connection  -----------------------*/
  String hostname = "Mon petit objet ESP32";
  // Credentials 
  String ssid = String("iPhone (2)");
  String passwd = String("e7znotn89aaxu");

  // !!!!!!!!!! Choose HERE the method to connect !!!!!!!!!
#define MULTI
  //-------------------------------------------------------
#ifdef BASIC
  // Way 1 : Basic connection
  wifi_connect_basic(hostname, ssid, passwd); 
#endif
#ifdef NEIGHBOR
  // Way 2 : Search Neighbor 
  int idx = wifi_search_neighbor();              
  if (idx != -1){ // and call the basic connexion 
    wifi_connect_basic(hostname, String(WiFi.SSID(idx)), passwd);      
  }
#endif
#ifdef MULTI 
  // Way 3 : Connection from a list of SSID 
  wifi_connect_multi(hostname);               
#endif
  
  /* WiFi status     --------------------------*/
  if (WiFi.status() == WL_CONNECTED){
    USE_SERIAL.print("\nWiFi connected : yes ! \n"); 
    wifi_printstatus(0);  
  } 
  else {
    USE_SERIAL.print("\nWiFi connected : no ! \n"); 
    //  ESP.restart();
  }
}
