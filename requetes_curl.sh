
# Changer par une IP correspondant à votre ESP32
ip="172.20.10.8"

# Requête permettant la communication entre l'ESP32, Node-Red et le tableau de bord
curl -X GET http://$ip/getJson #
curl -X POST http://$ip/target -d "ip=192.168.1.100&port=2872&sp=1234" # a tester
curl -X GET http://$ip/setNetwork 
curl -X POST http://$ip/setData -d 

# Requêtes permettant la mise à jour automatique de notre page web
curl -X GET http://$ip/temperature
curl -X GET http://$ip/light
curl -X GET http://$ip/cooler
curl -X GET http://$ip/heater
curl -X GET http://$ip/uptime
