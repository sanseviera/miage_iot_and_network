void mqtt_pubcallback(char* topic, 
                      byte* payload, 
                      unsigned int length) {
  /* 
   * Callback when a message is published on a subscribed topic.
   */
  USE_SERIAL.print("Message arrived on topic : ");
  USE_SERIAL.println(topic);
  USE_SERIAL.print("=> ");

  // Byte list (of the payload) to String and print to Serial
  String message;
  for (int i = 0; i < length; i++) {
    //USE_SERIAL.print((char)payload[i]);
    message += (char)payload[i];
  }
  USE_SERIAL.println(message);

  /*
  char msg[length + 1];
  memcpy(msg, payload, length);
  msg[length] = NULL;
  message = String(msg);
  */

  // Feel free to add more if statements to control more GPIOs with MQTT

  // If a message is received on the topic,
  // you check if the message is either "on" or "off".
  // Changes the output state according to the message
  if (String(topic) == parametre.topic_led) {
    USE_SERIAL.print("so ... changing output to ");
    if (message == "on") {
      USE_SERIAL.println("on");
      //set_LED(HIGH);
    }
    else if (message == "off") {
      USE_SERIAL.println("off");
      //set_LED(LOW);
    }
  }
}
