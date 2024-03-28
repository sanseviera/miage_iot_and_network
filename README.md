# miage_iot_and_network
Lors de notre master MIAGE, DIOP Serigne Rawane et BORREANI Théo avons réalisé un projet dans le cadre du cours "IoT & réseaux" de M. MENEZ Gilles. Le projet consiste en la réalisation d'un régulateur de température connecté avec un ESP32 et les langages C et Python.

## Collaborateurs 
* DIOP Serigne Rawane
* BORREANI Théo

## Améliorations du TP1
* Réduction d nombre d'onglets en passant de 5 à 3
* Correction du canal "Serial"
* Retour vers l'ESP : OKAY, on peut modifier la température d'alerte, la lumière d'alerte, le seuil haut et le seuil bas
* Ajout d'icones
* Ajout de graphiques d'évaluations des status


## Outils utilisés 

### Logiciels
* Arduino IDE 1.8.19
* C
* Python 3
* GitHub
* Node-RED
* HTML
* CSS 
* cURL

### Matériels 
* Une carte ESP32
* Des LEDs de couleurs
* Capteur de lumière
* Des résistances
* Un ventilateur
* Un thermomètre

## Documentation utilisateur

### Utilisation 

#### Arduino
1. Brancher l'ESP32 en respectant les broches.
1. Télécharger les bibliothèques nécessaires :
    * OneWire.h
    * DallasTemperature.h
    * Adafruit_NeoPixel.h
    * ArduinoJson.h
    * WiFi.h
    * WiFiMulti.h
    * SPIFFS.h
    * FS.h
    * ESPAsyncWebServer.h
    * ArduinoOTA.h
    * HTTPClient.h
1. Dans le code C principal "regul.ino", une structure appelée Parametre est disponible. Vous pouvez modifier les variables et constantes, __notament target_ip et target_port avec votre adresse IP local et le port sur lequel est lancé Node-RED__.
1. Dans le menu outil, téleverser les fichiers grâce à __ESP32 Sketch Data Upload__.
1. Téléverser le code sur l'ESP32 depuis l'IDE Arduino.
1. (Optionnel) Si vous rencontrez une erreur de compilation, recommencez en changeant la variable de préprocesseur "__Old__" à 1.

#### Validateur
1. Modifier le fichier exemple_1.json à votre guise
1. Executer val.py avec python3
```
python3 val.py
```

#### Tableau de bord
L'ESP32 sert également de serveur web. En recherchant dans un navigateur web l'adresse IP de l'ESP32, vous aurez accès à un tableau de bord qui charge périodiquement les données de l'ESP32. Vous aurez également accès à un formulaire permettant de changer les informations de connexion à Node-RED.
Pour que les donnéess soient accessible sur node-red, il faut entrer dans le formulaire votre adresse ip local de `Adresse IPv4` et non l'adresse ip du node-red.
De même l'ESP32 doit être ouvert ver l'ip du wifi (client) avec lequel vous vous connectez.

#### Script bash 

Un script shell est disponible pour tester les routes du serveur hébergés sur l'ESP32 via des commandes curl, vous pouvez modifier la variable ip avec l'IP qu'affichera votre ESP32 après la connexion.


#### Flux Node-RED
Nous vous avons fourni 3 flux node-red :
* ##### `Port_USB_Node-red.JSON` : qui contient les corrections et améliorations du rendu 1.
* ##### `Horodatage&http-request_Node-red.JSON` : qui représente une modélisation avec les noeuds http request qui récupère et/ou envoie les données du JSON grâce aux requêtes HTTP `getJson`et `setJson` et horodatage qui permet d'obtenir la requête `getJson` toutes les secondes et intervalles définies. Il fauut changer l'adresse IP des noeuds http request d'entrée et de sortie en celle de votre ESP.
* ##### `Http-in_http-response.JSON` : Le dashbord du rendu qui est configuré avec les noeuds `http in` et `http response` grâce à la requête http `/target` avec une connexion dynamique des adresses ip client et cible.


##### Vue d'ensemble
Ce flux Node-RED fournit un système complet pour la surveillance de divers paramètres environnementaux tels que la température, l'intensité lumineuse et la présence de feu. Il inclut également des fonctionnalités pour le rapport d'informations sur les appareils et le statut de connectivité.

##### Nœuds et Fonctionnalités

- **Entrée Série** : Écoute les données entrantes sur le port série.
- **Analyseur JSON** : Convertit le payload entrant en un objet JSON pour un traitement simplifié.
- **Nœuds Fonction** : Diverses fonctions pour traiter différentes données :
  - `temperature` : Extrait et enregistre les données de température.
  - `luminosité` : Extrait et enregistre l'intensité lumineuse.
  - `feuDetecte` : Vérifie la détection du feu et enregistre le statut.
  - `chaleur`, `froid` : Surveille le statut du chauffage et de la climatisation.
  - `SH`, `SB` : Surveille les seuils hauts et bas pour la régulation.
- **Graphiques** : Visualisent la température et l'intensité lumineuse dans le temps.
- **Leds** : Permettent le suivi et reflètent l'état de détection du feu, du chauffage, du régulateur et de la climatisation.
- **Jauges et Curseurs** : Affichent et contrôlent les seuils d'alerte de température.
- **Carte du Monde** : Montre la localisation géographique de l'appareil.
- **Modèles** : Éléments personnalisés de l'interface utilisateur pour afficher les données JSON et les informations de l'appareil de manière conviviale.
- **Notification** : Notifications pour alerter sur la détection du feu et le statut de la régulation :
    - Si le régulateur est etteint, il n'y a aucune notification.
    - Si le régulateur est allumé, chaque une minute une notification est envoyé en bas à droite pour dire que le régulateur est en marche.
    - Si aucune incendie n'est détectée, il n'y a aucune notification.
    - Si une incendie est détectée, à chaque fois que les données sont actualisées de l'ESP, une notification est envoyée.

##### Groupes de Tableau de Bord

- **Appareil&performance** : Informations sur la performance et sur les appareils.
- **Température&luminosité** : Suivi des variations des données de température et de luminosité, de même que la variation de ventilateur.
- **Alertes** : Gestion des alertes et notifications.
- **Localisation** : Fournit les informations sur le réseau et l'emplacement GPS de l'appareil.
- **JSON** : Section dédiée à la vérification du payload JSON.

##### Configuration et Installation

1. Importez le fichier de flux JSON dans votre instance Node-RED.
2. Configurez les connexions série et installez les bibliothèques nécessaires de Node-RED si elles ne sont pas déjà présentes.
3. Ajustez les éléments du tableau de bord comme les jauges et les graphiques en fonction de l'échelle de vos données.
4. Configurez les nœuds MQTT ou HTTP si vous transférez des données vers d'autres services (non inclus dans le fichier de flux).
5. Déployez le flux et accédez au tableau de bord Node-RED pour voir votre système de surveillance en action.

Note : Vous avez besoin des bibliothèques suivantes sur Node-RED :
- node-red
- node-red-contrib-ui-led
- node-red-contrib-web-worldmap
- node-red-dashboard
- node-red-node-serialport

## Autres


## Détaille de la gestion des incendies

L'ESP32 détecte si la valeur des variables lumière et chaleur atteint chacune un seuil haut particulier. Si c'est le cas, une variable représentant un pourcentage est augmentée ; sinon, cette variable est réduite. Si la probabilité de feu dépasse 80%, une alerte est déclenchée. Ce système a l'avantage de pouvoir évoluer au fil du temps.

## Correctifs par rapport au premier rendu 
* Ajout d'avantage de commentaires.
* Esthetique de l'interface Node-red amelioré.

## Les plus apporté
* Aucun délai n'est utilisé dans le code C. Au lieu de cela, nous utilisons des conditions et des variables qui nous permettent d'appeler des fonctions en choisissant indépendamment l'intervalle associé à chacune d'elles.
* Une carte intéractive disponible pour consulter la position théorique de l'ESP32.
* Une grande variété d'indicateurs.

<span style="color:#FFD700">
MERCI ET BONNE LECTURE !
</span>



