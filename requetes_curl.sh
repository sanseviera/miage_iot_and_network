#!/bin/bash

# Voici un script bash permettant de faire des requêtes curl pour communiquer avec l'ESP32
# Pour l'utiliser, il suffit de lancer le script bash dans un terminal ou 
# de copier coller les commandes dans un terminal.
# Assurez-vous que le script bash est exécutable avec la commande chmod +x requetes_curl.sh
# Assurez-vous que l'ESP32 est connecté au même réseau que
# votre ordinateur et que vous avez bien renseigné l'adresse IP de l'ESP32

# Changer par une IP correspondant à votre ESP32
ip="172.20.10.8"

# Requête permettant la communication entre l'ESP32, Node-Red et le tableau de bord
curl -X GET http://$ip/getJson #
curl -X POST http://$ip/target -d "ip=192.168.1.100&port=2872&sp=1234" 
curl -X POST http://$ip/setData -H "Content-Type: application/json" -d "{\"regul\":{\"lt\":10,\"ht\":20,\"temperatureAlerte\":30,\"lumiereAlerte\":2000}}"

# Requêtes permettant la mise à jour automatique de notre page web
curl -X GET http://$ip/temperature
curl -X GET http://$ip/light
curl -X GET http://$ip/cooler
curl -X GET http://$ip/heater
curl -X GET http://$ip/uptime
curl -X GET http://$ip/chancefeu
curl -X GET http://$ip/feu
curl -X GET http://$ip/tempmax
curl -X GET http://$ip/tempmin
curl -X GET http://$ip/vitesseventilateur
