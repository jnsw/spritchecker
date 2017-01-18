# spritchecker
Ein Spritpreischecker für einen ESP12 mit 1602 LCD-Display 

Ich fand es ziemlich spannend, ein LCD-Display den aktuell günstigstens Spritpreis in der Umgebung anzuzeigen und wo.

Dabei habe ich an der JSON-API von https://creativecommons.tankerkoenig.de/ bedient, die sich an der Markttransparenzstelle für Kraftstoffe (MTS-K), einer offiziellen Echtzeit-Quelle für Spritpreise, bedient.

Benötigte Libraries für Arduino:

ESP8266WiFi.h - Um Wifi auf dem ESP zu handlen

WiFiClientSecure.h - SSL Verbindung zu Tankerkönig

ArduinoJson - https://github.com/bblanchon/ArduinoJson

LiquidCrystal (in meinem Fall https://github.com/fdebrabander/Arduino-LiquidCrystal-I2C-library )

elapsedMillis - für das Updateintervall von 10min (man sollte nicht unter 4min gehen, da Tankerkönig eh nicht schneller aktualisiert)


Ist alles noch ziemlich unsauber, aber es funktioniert für's erste. Quasi Version 0.01



