# SilenceSensor
Csend szenzor építés
Egy napos Hackaton alkalmával a résztvevők az alábbi “Csend” szenzorokat építették meg
 
1.	Kép a kész szenzo, cseppmentes házában
 
2.	Kép Így nézett ki belülről
Ehhez az alábbi alkatrészeket és szolgáltatásokat használták fel:
 
-	Weemos D1 Mini – ESP8266
-	GY MAX4466 MIC
-	LED
-	220R
-	Tact nyomógomb
-	Jumper kábelek
-	Adafruit IO https://io.adafruit.com
-	WiFi manager https://github.com/tzapu/WiFiManager 
-	Arduino IDE
 

Az alábbi áramkört megépítve elkezdődhetett a programozás és az adatok küldése a felhőbe.
 
A szokásos Adafruit IO könyvtárat a későbbiekben megváltoztattuk, hogy a WiFi manager támogatásával kiegészítsük. Ezáltal könnyebbé vált a kész szenzor WiFi hálózatra való felcsatlakoztatása. 
Az alábbiakban az a kód részlet ami a D1 minikre is felkerült. Programozásra az Arduino IDE környezetet használták a résztvevők.
