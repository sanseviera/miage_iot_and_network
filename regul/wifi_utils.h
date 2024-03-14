/*** Basic/Static Wifi connection

     Fichier wificonnect/wifi_utils.h ***/

#include <WiFi.h> // https://www.arduino.cc/en/Reference/WiFi
#include <WiFiMulti.h>

#define SaveDisconnectTime 1000 // Connection may need several tries 
                                                          // Time in ms for save disconnection, => delay between try
                                                          // cf https://github.com/espressif/arduino-esp32/issues/2501  
#define WiFiMaxTry 10
String translateEncryptionType(wifi_auth_mode_t encryptionType);
void wifi_printstatus();
void wifi_connect_basic(String hostname, String ssid, String passwd);
int wifi_search_neighbor();
void wifi_connect_multi(String hostname);
