# miage_iot_and_network
Lors de notre master MIAGE, Serigne Rawane et moi-même avons réalisé un projet dans le cadre du cours "IoT & réseaux" de M. MENEZ Gilles. Le projet consiste en la réalisation d'un régulateur de température avec un ESP32 et les langages C et Python.

## Collaborateurs 
* DIOP Serigne Rawane
* BORREANI Théo

## Outils utilisés 

### Logiciels
* Arduino IDE 1.8.19
* C
* Python 3
* GitHub
* Node-RED

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
1. Téléverser le code sur l'ESP32 depuis l'IDE Arduino.
1. (Optionnel) Si vous avez une erreur de compilation, recommencez en changeant la variable de préprocesseur à 1.

#### Validateur
1. Modifier le fichier exemple_1.json à votre guise
1. Executer val.py avec python3
```
python3 val.py
```

#### Détaille de la gestion des incndies

L'ESP32 détecte si la valeur des variables lumière et chaleur atteint chacune un seuil haut particulier. Si c'est le cas, une variable représentant un pourcentage est augmentée ; sinon, cette variable est réduite. Si la probabilité de feu dépasse 80%, une alerte est déclenchée. Ce système a l'avantage de pouvoir évoluer au fil du temps.

#### Les plus apporté
* Aucun délai n'est utilisé dans le code C. Au lieu de cela, nous utilisons des conditions et des variables qui nous permettent d'appeler des fonctions en choisissant indépendamment l'intervalle associé à chacune d'elles.

MERCI ET BONNE LECTURE !



