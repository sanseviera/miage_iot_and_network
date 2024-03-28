

#define SaveDisconnectTime 1000 // Connection may need several tries 
                                                          // Time in ms for save disconnection, => delay between try
                                                          // cf https://github.com/espressif/arduino-esp32/issues/2501  
#define WiFiMaxTry 10
String translateEncryptionType(wifi_auth_mode_t encryptionType);
void wifi_printstatus();
void wifi_connect_multi(String hostname);
